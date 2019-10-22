//
// Created by moedinger on 17.10.19.
//

#include <iostream>
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
}// -------------------- NAMESPACE END --------------------------------


#include "nodedd.h"
