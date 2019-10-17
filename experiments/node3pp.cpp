//
// Created by moedinger on 17.10.19.
//

#include "node3pp.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------

    bool node3pp::addConnection(node &target, uint32_t tag) {
        // rework so dynamic_cast is not needed
        if(dynamic_cast<node3pp&>(target).requestConnection(*this, tag)) {
            this->requestConnection(target, tag);
        }
        return false;
    }

    bool node3pp::hasConnection(node &target, uint32_t tag) {
        switch (tag) {
            case networktag::broadcast:
                if (broadcast_connections.count(target.id) > 0)
                    return true;
            case networktag::group:
                if (group_connections.count(target.id) > 0)
                    return true;
        }
        return false;
    }

    size_t node3pp::amountConnections(uint32_t tag) {
        switch (tag) {
            case networktag::broadcast:
                return broadcast_connections.size();
            case networktag::group:
                return group_connections.size();
        }
        return 0;
    }

    void node3pp::receiveMessage(message m) {

    }

    void node3pp::startProtocol() {

    }

    bool node3pp::requestConnection(node &source, uint32_t tag) {
        if(hasConnection(source, tag))
            return true;

        switch(tag){
            case networktag::broadcast:
                broadcast_connections.emplace(source.id, source);
                return true;
            case networktag::group:
                group_connections.emplace(source.id, source);
                return true;
        }

        return false;
    }

} // -------------------- NAMESPACE END --------------------------------