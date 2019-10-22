//
// Created by moedinger on 17.10.19.
//

#include <iostream>
#include <cassert>
#include <fstream>
#include "nodedd.h"
#include "../../netsim_basic/net_edge_models.h"
#include "../../netsim_basic/net_creation.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------

    bool nodedd::requestConnection(node &source, uint32_t tag) {
        if(hasConnection(source, tag))
            return true;

        broadcast_connections.emplace(source.id, source);
        return true;
    }

    bool nodedd::addConnection(node &target, uint32_t tag) {

        // rework so dynamic_cast is not needed
        if(dynamic_cast<nodedd&>(target).requestConnection(*this, tag)) {
            this->requestConnection(target, tag);
        }
        return false;
    }

    bool nodedd::hasConnection(node &target, uint32_t tag) {
        if (broadcast_connections.count(target.id) > 0)
            return true;
        return false;
    }

    size_t nodedd::amountConnections(uint32_t tag) {
        return broadcast_connections.size();
    }

    void nodedd::receiveMessage(message m) {
        if(known_messages.count(m.payload) == 0) {
            known_messages.insert(m.payload);

            for(auto& conref : broadcast_connections) {
                auto& connection = conref.second.get();
                if(connection.id != m.from) {
                    message m2(this->id, connection.id, 0, m.payload);
                    net.sendMessage(m2);
                    sim.csv(std::to_string(m2.from)+","+std::to_string(m2.payload)+","+std::to_string(m2.to));
                }
            }

        }
    }

    void nodedd::startProtocol() {
        // create pseudorandom message payload
        message m(this->id, this->id, 0, (uint64_t(this->id)<<31)+uint32_t(rand()));
        sim.addEventIn([&](){
            this->receiveMessage(m);
        },0);
    }



    void runSimulationFAP() {
        const auto nodecount = 100000;
        const auto concount = 16;

        std::vector<std::function<void(std::vector<nodedd>&)>> strategies{uniformlyAtLeastK<nodedd,concount>};

        auto start = time(NULL);

        const std::string filename = std::to_string(nodecount)+"_k-"+std::to_string(concount)+"_FAP.csv";
        std::ofstream file;
        file.open (filename,std::ofstream::out | std::ofstream::trunc);
        file.close();
        file.open(filename,std::ios::app);

        network<nodedd> net(std::cout, file, strategies, nodecount, constModel<10>);

        auto id_limit = net.getNodeidLimit();
        assert(3<id_limit);
        net.startProtocolOn({1});

        net.runSimulation();
        file.close();

        std::cout << time(NULL) << ": Finished. It took " << time(NULL)-start << "s." << std::endl;
    }

}// -------------------- NAMESPACE END --------------------------------


#include "nodedd.h"
