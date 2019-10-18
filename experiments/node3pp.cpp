//
// Created by moedinger on 17.10.19.
//

#include <cmath>
#include "node3pp.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------
    namespace helper {
        template <typename C>
        void sendToGroup(basicnetwork& net, C group, uint32_t from, messagetype t, uint64_t payload) {
            for(auto& node: group){
                message m(from, node.id, t, payload);
                net.sendMessage(m);
            }
        }

        template <typename C>
        uint64_t closestID(C ids, const uint64_t m) {
            for(auto& id: ids) {
                id-=m;
            }
            return std::min(ids);
        }

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
        switch (m.messagetype) {
            case messagetype::dcinit:
                // falls schon gesehen, ignorieren
                // sonst sende commit1 an gruppe
                break;
            case messagetype::commit1:
                // falls noch kein dcinit erhalten wurde, löse selbst dcinit aus
                // falls alle commit1 der gruppe erhalten, sende dining1
                break;
            case messagetype::dining1:
                // falls alle dining1 erhalten wurde, sende dining2
                break;
            case messagetype::dining2:
                // falls alle dining2 erhalten wurde, sende commit2
                break;
            case messagetype::commit2:
                // falls alles commit2 erhalten wurden, sende dining3
                break;
            case messagetype::dining3:
                // falls alles dining3 erhalten wurden, sende dining4
                break;
            case messagetype::dining4:
                // falls alle dining4 erhalten wurde
                //     überprüfe ob verantwortlich, falls ja starte AD
                //     falls nein, tue nichts
                break;
            case messagetype::vsource:
                // complicated
                break;
            case messagetype::adaptive:
                // falls noch keine N gewählt, wähle N nachbarn
                // falls bereits gewählt, sende an N nachbarn
                break;
            case messagetype::flood:
                // falls unbekannt, leite an alle nachbarn außer sender weiter
                break;
        }
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