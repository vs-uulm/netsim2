//
// Created by moedinger on 17.10.19.
//

#ifndef NETSIM2_NODE3PP_H
#define NETSIM2_NODE3PP_H

#include "../../simulator.h"
#include "../../net_network.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------

// we will need 2 network layers
namespace pp {
    enum networktag {
        broadcast,
        group
    };

    enum messagetype {
        dcinit,
        commit1,
        dining1,
        dining2,
        commit2,
        dining3,
        dining4,
        adaptive,
        vsource,
        flood
    };
}

    class node3pp {
    private:
        struct AD {
            uint64_t step;
            uint64_t h;
        };

        // used simulator
        simulator& sim;

        // used network
        basicnetwork& net;

        bool closestID(uint64_t id);
        std::unordered_map<uint64_t, AD> ad_stats;
        void selectN(uint32_t n, uint32_t ignore, uint64_t payload);
        std::unordered_map<uint64_t,std::vector<std::reference_wrapper<node3pp>>> selected_n;

        std::unordered_map<uint64_t, nodeid> known_messages;

        std::unordered_map<uint64_t, std::unordered_map<uint32_t, uint32_t>> phase_recorder;

        std::unordered_map<nodeid, std::reference_wrapper<node3pp>> broadcast_connections;
        std::unordered_map<nodeid, std::reference_wrapper<node3pp>> group_connections;

        bool haveAllSent(uint32_t received_messagetype, uint64_t payload);
        void whenAllSend(uint32_t received_messagetype, uint32_t tosend_messagetype, uint64_t payload);

        bool requestConnection(node3pp &source, uint32_t tag);

    public:
        static uint32_t d;
        static uint32_t N;

        // substitute for ip adress
        const uint32_t id;

        // creation by moving or new is allowed
        node3pp(basicnetwork& net, simulator& sim, nodeid id) : net(net), sim(sim), id(id) {};
        // moving will keep the ID intact
        node3pp(node3pp&& n) noexcept : net(n.net), sim(n.sim), id(n.id) {};

        // Management overrides
        bool addConnection(node3pp &target, uint32_t tag);

        bool hasConnection(node3pp &n, uint32_t tag);

        size_t amountConnections(uint32_t tag);

        // protocol overrides
        void receiveMessage(message m);

        void startProtocol();

        const bool hasSeen(uint64_t payload) const { return known_messages.count(payload); }
    };


} // -------------------- NAMESPACE END --------------------------------
#endif //NETSIM2_NODE3PP_H
