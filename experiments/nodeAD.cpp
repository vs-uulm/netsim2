//
// Created by moedinger on 21.10.19.
//

#include <fstream>
#include "nodeAD.h"
#include "../netsim_basic/net_edge_models.h"
#include "../netsim_basic/net_creation.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------
    namespace helper {
        constexpr double p(const uint16_t s, const uint16_t h, const uint16_t d) {
            if(d==2)
                return (static_cast<double>(s-2*h+2))/(s+2);
            return (pow((d-1),(s/2-h+1))-1)/(pow((d-1),(s/2+1))-1);
        }
    }

    bool nodeAD::requestConnection(node &source, uint32_t tag) {
        if(hasConnection(source, tag))
            return true;

        broadcast_connections.emplace(source.id, source);
        return true;
    }

    bool nodeAD::addConnection(node &target, uint32_t tag) {

        // rework so dynamic_cast is not needed
        if(dynamic_cast<nodeAD&>(target).requestConnection(*this, tag)) {
            this->requestConnection(target, tag);
        }
        return false;
    }

    bool nodeAD::hasConnection(node &target, uint32_t tag) {
        if (broadcast_connections.count(target.id) > 0)
            return true;
        return false;
    }

    size_t nodeAD::amountConnections(uint32_t tag) {
        return broadcast_connections.size();
    }

    void nodeAD::receiveMessage(message m) {
        switch(m.messagetype) {
            case messagetype::diffuse:
                if(known_messages.count(m.payload) > 0) {
                    if(m.from == known_messages[m.payload]){
                        for(auto& node : broadcast_connections) {
                            if(node.second.get().id != m.from){
                                message m2(this->id, node.second.get().id, m.messagetype, m.payload);
                                net.sendMessage(m2);
                            }
                        }
                    }
                }else{
                    known_messages[m.payload] = m.from;
                }
                break;

            case messagetype::vsource:
                const auto diffusepayload = m.payload & 0x00000000FFFFFFFF;

                for(auto& node: broadcast_connections){
                    if(node.second.get().id != m.from){
                        message m2(this->id,node.second.get().id,messagetype::diffuse, diffusepayload);
                        net.sendMessage(m2);
                    }
                }

                AD& ad = ad_stats[diffusepayload];
                ad.h = ((m.payload & 0xFFFF000000000000) >> 48);
                ad.step = (m.payload & 0x0000FFFF00000000) >> 32;

                ad.h += 1;

                auto loop = [](auto& func, const message m, nodeAD& self) mutable ->void{
                    const auto diffusepayload = m.payload & 0x00000000FFFFFFFF;
                    AD& ad = self.ad_stats[diffusepayload];

                    if(ad.step<=d){
                        std::minstd_rand gen(std::random_device{}());
                        std::uniform_real_distribution<double> U(0,1);

                        ad.step += 1;
                        if(helper::p(ad.step-1,ad.h, d) <= U(gen)) {
                            for(auto& node: self.broadcast_connections){
                                message m2(self.id, node.second.get().id, messagetype::diffuse,diffusepayload);
                                self.net.sendMessage(m2);
                            }
                        }else{
                            // select random connection that is not the previous vsource token
                            std::uniform_int_distribution<> dist(0,self.broadcast_connections.size()-1);
                            decltype(broadcast_connections.begin()) it;
                            do{
                                it = self.broadcast_connections.begin();
                                std::advance(it, dist(gen));
                            }while(it->second.get().id==m.from);

                            const uint64_t vsourcepayload = (uint64_t(ad.h) << 48) + (uint64_t(ad.step) << 32) + diffusepayload;
                            message m2(self.id,it->second.get().id,messagetype::vsource,vsourcepayload);
                            self.net.sendMessage(m2);
                            self.known_messages[diffusepayload] = it->second.get().id;
                            return;
                        }
                        self.sim.addEventIn([&func, m, &self](){func(func, m, self);},15);
                        return;
                    }
                };
                loop(loop, m, *this);
                break;
        }
    }

    void nodeAD::startProtocol() {
        message m(this->id, this->id, messagetype::vsource, 0);
        known_messages[0] = this->id;
        receiveMessage(m);
    }



    void runSimulationAD() {
        const auto nodecount = 50;
        const auto concount = 4;

        std::vector<std::function<void(std::vector<nodeAD>&)>> strategies{uniformlyAtLeastK<nodeAD,concount>};

        auto start = time(NULL);


        const std::string filename = std::to_string(nodecount)+"_k-"+std::to_string(concount)+"_AD.csv";
        std::ofstream file;
        file.open (filename,std::ofstream::out | std::ofstream::trunc);
        file.close();
        file.open(filename,std::ios::app);

        network<nodeAD> net(std::cout, file, strategies, nodecount, constModel<10>);

        net.startProtocolOn({1});

        net.runSimulation();
        file.close();

        std::cout << time(NULL) << ": Finished. It took " << time(NULL)-start << "s." << std::endl;
    }

}// -------------------- NAMESPACE END --------------------------------

