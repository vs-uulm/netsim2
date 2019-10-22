//
// Created by moedinger on 17.10.19.
//

#ifndef NETSIM2_nodedd_H
#define NETSIM2_nodedd_H


#include <unordered_set>
#include "../../netsim_basic/net_nodes.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------

    enum dd_messagetype {
        stem,
        proapagte
    };

    class nodedd : public node {
    private:

        std::unordered_set<uint64_t> known_messages;
        std::unordered_map<nodeid, std::reference_wrapper<node>> broadcast_connections;

    protected:
        bool requestConnection(node &source, uint32_t tag) override;

    public:
        static double p_phasechange;

        using node::node;

        bool addConnection(node &target, uint32_t tag) override;

        bool hasConnection(node &target, uint32_t tag) override;

        size_t amountConnections(uint32_t tag) override;

        void receiveMessage(message m) override;

        void startProtocol() override;

    };
}// -------------------- NAMESPACE END --------------------------------

#endif //NETSIM2_nodedd_H
