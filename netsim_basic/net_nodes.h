//
// Created by moedinger on 16.10.19.
//

#ifndef NETSIM2_NET_NODES_H
#define NETSIM2_NET_NODES_H

#include "../simulator.h"
#include "../net_network.h"

/**
 * General node type, to implement a protocol,
 * derive a subclass and implement the pure virtual functions for management and communication.
 *
 * Add using node::node or add a constructor with the same signature to use in the network class.
 */
class node {
protected:
    // used simulator
    simulator& sim;

    // used network
    basicnetwork& net;

    // internal method if connections need to be bidirectional
    virtual bool requestConnection(node& source, uint32_t tag) = 0;

public:
    // substitute for ip adress
    const uint32_t id;

    // to keep IDs unique, copying is not allowed
    node(const node& n) = delete;

    // creation by moving or new is allowed
    node(basicnetwork& net, simulator& sim, nodeid id) : net(net), sim(sim), id(id) {};
    // moving will keep the ID intact
    node(node&& n) noexcept : net(n.net), sim(n.sim), id(n.id) {};

    virtual ~node() = default;

    // Management protocol specific functions
    // tags allow for different connection buckets
    virtual bool addConnection(node& target, uint32_t tag) = 0;
    virtual bool hasConnection(node& target, uint32_t tag) = 0;
    virtual size_t amountConnections(uint32_t tag) = 0;

    // Communication protocol logic
    virtual void receiveMessage(message m) = 0;
    virtual void startProtocol() = 0;
};

#endif //NETSIM2_NET_NODES_H
