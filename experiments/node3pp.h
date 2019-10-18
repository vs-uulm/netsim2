//
// Created by moedinger on 17.10.19.
//

#ifndef NETSIM2_NODE3PP_H
#define NETSIM2_NODE3PP_H


#include "../netsim_basic/net_nodes.h"

namespace experiments {
    // -------------------- NAMESPACE BEGIN --------------------------------

// we will need 2 network layers
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

    class node3pp : public node {
    private:

        std::unordered_map<nodeid, std::reference_wrapper<node>> broadcast_connections;
        std::unordered_map<nodeid, std::reference_wrapper<node>> group_connections;

    protected:
        bool requestConnection(node &source, uint32_t tag) override;

    public:
        using node::node;

        // Management overrides
        bool addConnection(node &target, uint32_t tag) override;

        bool hasConnection(node &n, uint32_t tag) override;

        size_t amountConnections(uint32_t tag) override;

        // protocol overrides
        void receiveMessage(message m) override;

        void startProtocol() override;
    };


} // -------------------- NAMESPACE END --------------------------------
#endif //NETSIM2_NODE3PP_H
