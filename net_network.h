//
// Created by moedinger on 16.10.19.
//

#ifndef NETSIM2_NET_NETWORK_H
#define NETSIM2_NET_NETWORK_H

#include <iostream>
#include "simulator.h"

using nodeid = uint32_t;

struct message {
    const nodeid from;
    const nodeid to;
    const uint32_t messagetype;
    const uint64_t payload;

    message(nodeid from, nodeid to, uint32_t messagetype, uint64_t payload) : from(from), to(to), messagetype(messagetype), payload(payload) {};
};

/**
 * basic network interface used by nodes
 */
class basicnetwork {
public:
    virtual void sendMessage(message m) = 0;
};

#include "netsim_basic/net_nodes.h"

/**
 * Main network implementation for managing the network
 * @tparam Node type of nodes in the network, the node type must support the constructor signature of the basic node class
 */
template<typename Node>
class network : public basicnetwork {
private:
    // node management
    std::vector<Node> nodes;

    // modelling all the edges between nodes
    std::function<double(void)> edgemodel;

public:
    simulator sim;

    const std::vector<Node>& getNodes() const {return nodes;}

    /**
     * creates nodes and links between those nodes.
     */
    template<typename strategies>
    network(bool verbosity, std::vector<strategies> creators, uint32_t size, decltype(edgemodel) edgemodel) : network(std::cout, std::cout, creators, size, edgemodel) {
        sim.csv_verbosity = verbosity;
        sim.log_verbosity = verbosity;
    }

    template<typename strategies>
    network(std::ostream& log,
            std::ostream& csv,
            std::vector<strategies> creators,
            uint32_t size,
            decltype(edgemodel) edgemodel) : sim(log, csv) {
        this->edgemodel = edgemodel;

        basicnetwork& basenet = *this;

        // generate the nodes
        for(auto i=0; i<size; i+=1) {
            nodeid temp_id = static_cast<nodeid>(nodes.size());
            nodes.emplace_back(basenet, sim, temp_id);
        }

        // run network creators to build connections
        for(auto creator : creators) {
            creator(nodes);
        }
    }

    /**
     * Send a message with a delay based on the edgemodel
     */
    void sendMessage(message m) {
        if(m.to < nodes.size()) {
            sim.csv(std::to_string(m.from)+","+std::to_string(m.to)+(m.payload!=0?","+std::to_string((m.payload & 0x0000ffff00000000)>>32)+","+std::to_string((m.payload & 0xffff000000000000)>>48):""));
            auto& node = nodes[m.to];
            sim.addEventIn([&node,m](){
                node.receiveMessage(m);
            },edgemodel());
        }else{
            // error: invalid target
        }
    }

    /**
     * starts the protocol on a given number of nodes
     */
    void startProtocolOn(std::vector<nodeid> nodes) {
        for(auto node : nodes) {
            this->nodes[node].startProtocol();
        }
    }

    /**
     * starts the protocol on all nodes
     */
    void startProtocolOnAll() {
        for(auto& node : nodes) {
            node.startProtocol();
        }
    }

    size_t getNodeidLimit() {
        return this->nodes.size();
    }

    /**
     * main function to run the simulation
     */
    void runSimulation(uint64_t maxtime = uint64_t(1)<<63) {
        while (!sim.empty() && sim.now() < maxtime)
            sim.next();
    }
};

#endif //NETSIM2_NET_NETWORK_H
