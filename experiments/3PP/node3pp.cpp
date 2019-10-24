//
// Created by moedinger on 17.10.19.
//

#include <cmath>
#include <random>
#include <chrono>
#include "node3pp.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------

    namespace pp {
        uint32_t flooding_received = ~0;
    }

    // static initializer
    uint32_t node3pp::d = 3;
    uint32_t node3pp::N = 3;

    namespace helper {
        constexpr double p(const uint16_t s, const uint16_t h, const uint16_t d) {
            if(d==2)
                return (static_cast<double>(s-2*h+2))/(s+2);
            return (pow((d-1),(s/2-h+1))-1)/(pow((d-1),(s/2+1))-1);
        }
    }

    bool node3pp::addConnection(node &target, uint32_t tag) {
        // rework so dynamic_cast is not needed
        if(dynamic_cast<node3pp&>(target).requestConnection(*this, tag)) {
            this->requestConnection(target, tag);
        }
        return false;
    }

    bool node3pp::hasConnection(node &target, uint32_t tag) {
        switch (tag) {
            case pp::networktag::broadcast:
                if (broadcast_connections.count(target.id) > 0)
                    return true;
            case pp::networktag::group:
                if (group_connections.count(target.id) > 0)
                    return true;
        }
        return false;
    }

    size_t node3pp::amountConnections(uint32_t tag) {
        switch (tag) {
            case pp::networktag::broadcast:
                return broadcast_connections.size();
            case pp::networktag::group:
                return group_connections.size();
        }
        return 0;
    }

    bool node3pp::haveAllSent(uint32_t received_messagetype, uint64_t payload) {
        return phase_recorder[payload][received_messagetype] >= group_connections.size();
    }

    void node3pp::whenAllSend(uint32_t received_messagetype, uint32_t tosend_messagetype, uint64_t payload) {
        phase_recorder[payload][received_messagetype] += 1;
        if(haveAllSent(received_messagetype, payload)){
            for(auto& node : group_connections) {
                message m(id, node.second.get().id, tosend_messagetype, payload);
                net.sendMessage(m);
            }
        }
    }

    void node3pp::selectN(uint32_t n, uint32_t ignore, uint64_t payload) {
        std::vector<std::reference_wrapper<node>> selected;

        // all legal participants
        for(auto& node : broadcast_connections) {
            if(node.second.get().id != ignore){
                selected.push_back(node.second);
            }
        }

        // shuffle and remove elements until its not too many anymore
        auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(selected.begin(), selected.end(),std::default_random_engine(seed));
        while(selected.size()>N){
            selected.pop_back();
        }

        selected_n[payload] = selected;
    }

    bool node3pp::closestID(uint64_t payload) {
        const auto mydistance = payload ^ id;
        for(auto& node: group_connections) {
            if(payload ^ node.second.get().id < mydistance)
                return false;
        }
        return true;
    }

    void node3pp::receiveMessage(message m) {
        switch (m.messagetype) {
            case pp::messagetype::dcinit:
                if(phase_recorder[m.payload][pp::messagetype::dcinit]!=0)
                    break;

                phase_recorder[m.payload][pp::messagetype::dcinit] = 1;
                for(auto& node : group_connections) {
                    message m2(id, node.second.get().id, pp::messagetype::commit1, m.payload);
                    net.sendMessage(m2);
                }
                break;
            case pp::messagetype::commit1:
                if(phase_recorder[m.payload][pp::messagetype::dcinit] == 0) {
                    message m2(id, id, pp::messagetype::dcinit, m.payload);
                    receiveMessage(m2);
                }
                whenAllSend(m.messagetype, pp::messagetype::dining1, m.payload);
                break;
            case pp::messagetype::dining1:
                whenAllSend(m.messagetype, pp::messagetype::dining2, m.payload);
                break;
            case pp::messagetype::dining2:
                whenAllSend(m.messagetype, pp::messagetype::commit2, m.payload);
                break;
            case pp::messagetype::commit2:
                whenAllSend(m.messagetype, pp::messagetype::dining3, m.payload);
                break;
            case pp::messagetype::dining3:
                whenAllSend(m.messagetype, pp::messagetype::dining4, m.payload);
                break;
            case pp::messagetype::dining4:
                phase_recorder[m.payload][pp::messagetype::dining4] += 1;
                if(haveAllSent(pp::messagetype::dining4, m.payload)) {
                    if(closestID(m.payload)) {
                        message m2(id, id, pp::messagetype::vsource, m.payload);
                        receiveMessage(m2);
                    }
                }
                break;

            case pp::messagetype::vsource: {
                const auto diffusepayload = m.payload & 0x00000000FFFFFFFF;

                const auto tmp_N = m.from == id ? N : N-1;
                if(selected_n[diffusepayload].size() == 0)
                    selectN(tmp_N, m.from, diffusepayload);

                for (auto &node: selected_n[diffusepayload]) {
                    if (node.get().id != m.from) {
                        message m2(id, node.get().id, pp::messagetype::adaptive, diffusepayload);
                        net.sendMessage(m2);
                    }
                }

                AD &ad = ad_stats[diffusepayload];
                ad.h = ((m.payload & 0xFFFF000000000000) >> 48);
                ad.step = (m.payload & 0x0000FFFF00000000) >> 32;

                ad.h += 1;

                auto loop = [](auto &func, const message m, node3pp &self) mutable -> void {
                    const auto diffusepayload = m.payload & 0x00000000FFFFFFFF;
                    AD &ad = self.ad_stats[diffusepayload];

                    if (ad.step <= d) {
                        std::minstd_rand gen(std::random_device{}());
                        std::uniform_real_distribution<double> U(0, 1);

                        ad.step += 1;
                        if (helper::p(ad.step - 1, ad.h, d) <= U(gen)) {
                            for (auto &node: self.selected_n[diffusepayload]) {
                                message m2(self.id, node.get().id, pp::messagetype::adaptive, diffusepayload);
                                self.net.sendMessage(m2);
                            }
                        } else {
                            // select random connection that is not the previous vsource token
                            std::uniform_int_distribution<> dist(0, self.broadcast_connections.size() - 1);
                            decltype(selected_n[diffusepayload].begin()) it;
                            do {
                                it = self.selected_n[diffusepayload].begin();
                                std::advance(it, dist(gen));
                            } while (it->get().id == m.from);

                            const uint64_t vsourcepayload =
                                    (uint64_t(ad.h) << 48) + (uint64_t(ad.step) << 32) + diffusepayload;
                            message m2(self.id, it->get().id, pp::messagetype::vsource, vsourcepayload);
                            self.net.sendMessage(m2);
                            self.known_messages[diffusepayload] = it->get().id;
                            return;
                        }
                        self.sim.addEventIn([&func, m, &self]() { func(func, m, self); }, 15);
                        return;
                    }else{
                        // compared to regular AD we need to start flooding in the end
                        message m_toflooding(self.id, self.id, pp::messagetype::flood, diffusepayload);
                        self.receiveMessage(m_toflooding);
                    }
                };
                loop(loop, m, *this);
                break;
            }

            case pp::messagetype::adaptive:
                if(known_messages.count(m.payload) > 0) {
                    if(m.from == known_messages[m.payload] && known_messages[m.payload] != pp::flooding_received){
                        for(auto& node : selected_n[m.payload]) {
                            if(node.get().id != m.from){
                                message m2(this->id, node.get().id, m.messagetype, m.payload);
                                net.sendMessage(m2);
                            }
                        }
                    }
                }else{
                    if(selected_n[m.payload].size() == 0)
                        selectN(N, m.from, m.payload);
                    known_messages[m.payload] = m.from;
                }
                break;

            case pp::messagetype::flood:
                if(known_messages[m.payload] == pp::flooding_received)
                    break;
                known_messages[m.payload] = pp::flooding_received;
                for(auto& node : broadcast_connections) {
                    if(node.second.get().id != m.from) {
                        message m2(id, node.second.get().id, pp::messagetype::flood, m.payload);
                        net.sendMessage(m2);
                    }
                }
                break;
        }
    }

    void node3pp::startProtocol() {
        message m(id, id, pp::messagetype::dcinit, 0);
        known_messages[0] = id;
        sim.addEventIn([&,m](){
            this->receiveMessage(m);
            for(auto& node : this->group_connections) {
                message m2(node.second.get().id, node.second.get().id, pp::messagetype::dcinit, 0);
                node.second.get().receiveMessage(m2);
            }
        },0);
    }

    bool node3pp::requestConnection(node &source, uint32_t tag) {
        if(hasConnection(source, tag))
            return true;

        switch(tag){
            case pp::networktag::broadcast:
                broadcast_connections.emplace(source.id, source);
                return true;
            case pp::networktag::group:
                group_connections.emplace(source.id, source);
                return true;
        }

        return false;
    }

} // -------------------- NAMESPACE END --------------------------------