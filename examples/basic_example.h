//
// Created by moedinger on 16.10.19.
//

#include <cassert>
#include "../netsim_basic/net_nodes.h"
#include "../netsim_basic/net_creation.h"
#include "../netsim_basic/net_edge_models.h"

namespace examples {
    // -------------------- NAMESPACE BEGIN --------------------------------

    // To create our protocol for the network we have to derive a node from the basic node class
    class mynode : public node {
    private:
        // our node has to do all the connection management itself
        std::unordered_map<nodeid,std::reference_wrapper<node>> connections;
    public:
        // We need to provide the same signature for our constructor as the base class
        // as we do not need to perform any onther action during construction we just reuse the base constructur.
        using node::node;
        // alternatively specify:
        // mynode(basicnetwork& net, simulator& sim, nodeid id) : node(net, sim, id) {
        //
        // }
    protected:
        virtual bool requestConnection(node &source, uint32_t tag) override {
            return true;
        }

    public:

        // Management protocol specific functions
        // tags allow for different connection buckets
        virtual bool addConnection(node& target, uint32_t tag) override {
            // this will generate a directed graph
            // as nodes are not informed of them receiving a connection
            // we would have to call requestConnection
            if(this->hasConnection(target, tag))
                return false;
            connections.emplace(target.id, target);
            return true;
        };
        virtual bool hasConnection(node& target, uint32_t tag) override {
            if(connections.count(target.id)>0)
                return true;
            return false;
        };
        virtual size_t amountConnections(uint32_t tag) override {
            return connections.size();
        };

        // Communication protocol logic
        virtual void receiveMessage(message m) override {
            // We create a simple TTL message thats forwarded to the first entry in the connections bucket
            sim.log(std::to_string(m.to)+" received message from "+std::to_string(m.from)+" with a TTL of "+std::to_string(m.payload));
            if(m.payload>0 && m.messagetype == 0) {
                // We do not want to send it instantly, we want to send it after a delay of 200ms
                auto delay = 200;

                // we create the message so we do not have to caputre this or more than neccessary
                message m2(this->id, this->connections.begin()->second.get().id,0,m.payload-1);
                basicnetwork& network = net;

                sim.addEventIn(
                        [&network,m2](){
                            network.sendMessage(m2);
                            }
                        ,delay);
            }
        };
        virtual void startProtocol() override {
            message m(this->id,this->connections.begin()->second.get().id,0,5);
            net.sendMessage(m);
        };

    };

    void runSimulation() {
        // first we create a vector of all network creation strategies we want to use and their respective tags, although our network will ignore the tag
        std::vector<std::function<void(std::vector<mynode>&)>> strategies{uniformlyAtLeastK<mynode,3,0>};

        // We use this to create a simple network of 100 nodes and a constant edge delay (delay introduced for each message sent) of 10ms
        network<mynode> net(true, strategies, 100, constModel<10>);

        // We initialize the protocol on the first 4 nodes and make sure there are at least 4 nodes
        //auto id_limit = net.getNodeidLimit();
        //assert(3<id_limit);
        //net.startProtocolOn({0, 1,2,3});

        // alternatively call startProtocolOnAll
        net.startProtocolOnAll();

        // finally we run the simulator, as we are sure it shouldn't take too long, we will not specify a maxtime
        net.runSimulation();
    }
} // -------------------- NAMESPACE END --------------------------------