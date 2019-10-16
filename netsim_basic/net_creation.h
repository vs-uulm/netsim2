//
// Created by moedinger on 16.10.19.
//

#ifndef NETSIM2_NET_CREATION_H
#define NETSIM2_NET_CREATION_H

#include <cstdint>
#include <vector>
#include <random>

/**
 * Example network creator scheme that ensures at least k connections are created for each node.
 * More connections are very likely for nodes earlier in the vector.
 */
template <typename T, uint16_t k, uint32_t tag=0>
void uniformlyAtLeastK(std::vector<T>& nodes){
    auto maximum = k<nodes.size()-1?nodes.size()-1:k;

    static std::minstd_rand gen(std::random_device{}());
    static std::uniform_int_distribution<uint32_t> dist(0, nodes.size()-1);

    for(auto& host: nodes) {
        while(host.amountConnections(tag)<maximum) {
            auto rnd = dist(gen);
            host.addConnection(nodes[rnd],tag);
        }
    }
}

#endif //NETSIM2_NET_CREATION_H
