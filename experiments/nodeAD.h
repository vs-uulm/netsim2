//
// Created by moedinger on 21.10.19.
//

#ifndef NETSIM2_NODEAD_H
#define NETSIM2_NODEAD_H

#include <unordered_set>
#include "../netsim_basic/net_nodes.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------

    enum messagetype {
        diffuse,
        vsource
    };

    const auto d = 3;

    class nodeAD : public node {
    private:
        struct AD {
            uint64_t step;
            uint64_t h;
        };

        std::unordered_map<uint64_t, AD> ad_stats;

        std::unordered_map<uint64_t, nodeid> known_messages;
        std::unordered_map<nodeid, std::reference_wrapper<node>> broadcast_connections;

    protected:
        bool requestConnection(node &source, uint32_t tag) override;

    public:
        using node::node;

        bool addConnection(node &target, uint32_t tag) override;

        bool hasConnection(node &target, uint32_t tag) override;

        size_t amountConnections(uint32_t tag) override;

        void receiveMessage(message m) override;

        void startProtocol() override;

    };

    void runSimulationAD();
}// -------------------- NAMESPACE END --------------------------------

#endif //NETSIM2_NODEAD_H
