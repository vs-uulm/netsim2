//
// Created by moedinger on 17.10.19.
//

#ifndef NETSIM2_NODE3PP_H
#define NETSIM2_NODE3PP_H


#include "../../netsim_basic/net_nodes.h"

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

    class node3pp : public node {
    private:
        struct AD {
            uint64_t step;
            uint64_t h;
        };

        bool closestID(uint64_t id);
        std::unordered_map<uint64_t, AD> ad_stats;
        void selectN(uint32_t n, uint32_t ignore, uint64_t payload);
        std::unordered_map<uint64_t,std::vector<std::reference_wrapper<node>>> selected_n;

        std::unordered_map<uint64_t, nodeid> known_messages;

        std::unordered_map<uint64_t, std::unordered_map<uint32_t, uint32_t>> phase_recorder;

        std::unordered_map<nodeid, std::reference_wrapper<node>> broadcast_connections;
        std::unordered_map<nodeid, std::reference_wrapper<node>> group_connections;

        bool haveAllSent(uint32_t received_messagetype, uint64_t payload);
        void whenAllSend(uint32_t received_messagetype, uint32_t tosend_messagetype, uint64_t payload);
    protected:
        bool requestConnection(node &source, uint32_t tag) override;

    public:
        static uint32_t d;
        static uint32_t N;

        using node::node;

        // Management overrides
        bool addConnection(node &target, uint32_t tag) override;

        bool hasConnection(node &n, uint32_t tag) override;

        size_t amountConnections(uint32_t tag) override;

        // protocol overrides
        void receiveMessage(message m) override;

        void startProtocol() override;

        const bool hasSeen(uint64_t payload) const { return known_messages.count(payload); }
    };


} // -------------------- NAMESPACE END --------------------------------
#endif //NETSIM2_NODE3PP_H
