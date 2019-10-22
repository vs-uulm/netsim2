//
// Created by moedinger on 17.10.19.
//

#include <iostream>
#include <random>
#include "nodedd.h"
#include "../../netsim_basic/net_edge_models.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------

    // static initializer
    double nodedd::p_phasechange = 0.1;

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
        switch (m.messagetype) {
            case dd_messagetype::propagate:
                // normal FAP
                if(known_messages.count(m.payload) == 0) {
                    known_messages.insert(m.payload);

                    for(auto& conref : broadcast_connections) {
                        auto& connection = conref.second.get();
                        if(connection.id != m.from) {
                            message m2(this->id, connection.id, dd_messagetype::propagate, m.payload);
                            net.sendMessage(m2);
                            sim.csv(std::to_string(m2.from)+","+std::to_string(m2.payload)+","+std::to_string(m2.to));
                        }
                    }

                }
                break;
            case dd_messagetype::stem:
                std::minstd_rand gen(std::random_device{}());
                std::uniform_real_distribution<double> U(0,1);


                if(U(gen)<=p_phasechange){
                    message m2(id, id, dd_messagetype::propagate, m.payload);
                    this->receiveMessage(m2);
                }else{
                    std::uniform_int_distribution<> dist(0,this->broadcast_connections.size()-1);
                    decltype(broadcast_connections.begin()) it;
                    do{
                        it = this->broadcast_connections.begin();
                        std::advance(it, dist(gen));
                    }while(it->second.get().id==m.from);
                    message m2(id, it->second.get().id, dd_messagetype::stem, m.payload);
                    net.sendMessage(m2);
                }
                break;
        }
    }

    void nodedd::startProtocol() {
        // create pseudorandom message payload
        message m(this->id, this->id, 0, 0);
        sim.addEventIn([&,m](){
            this->receiveMessage(m);
        },0);
    }
}// -------------------- NAMESPACE END --------------------------------


#include "nodedd.h"
