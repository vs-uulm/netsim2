//
// Created by moedinger on 16.10.19.
//

#ifndef NETSIM2_NET_CREATION_H
#define NETSIM2_NET_CREATION_H

#include <cstdint>
#include <vector>
#include <random>
#include "net_nodes.h"

template<typename Node>
class abstractNetCreation {
public:
    virtual void operator()(std::vector<Node>& nodes) = 0;
    virtual ~abstractNetCreation() = default;
};

template<typename Node>
class uniformlyAtLeastK : public abstractNetCreation<Node> {
private:
    uint32_t K, tag;
public:
    explicit uniformlyAtLeastK(uint32_t K, uint32_t tag = 0) : K(K), tag(tag) {}

    void operator()(std::vector<Node>& nodes) override {
        auto maximum = K>nodes.size()-1?nodes.size()-1:K;

        static std::minstd_rand gen(std::random_device{}());
        static std::uniform_int_distribution<uint32_t> dist(0, nodes.size()-1);

        for(auto& host: nodes) {
            while(host.amountConnections(tag)<maximum) {
                auto rnd = dist(gen);
                host.addConnection(nodes[rnd],tag);
            }
        }
    }
};

template<typename Node>
class groupOfSizeK : abstractNetCreation<Node> {
private:
    uint32_t K, tag;
public:
    explicit groupOfSizeK(uint32_t K, uint32_t tag = 0) : K(K), tag(tag) {}

    void operator()(std::vector<Node>& nodes) override {
        auto maximum = K>nodes.size()-1?nodes.size()-1:K;

        std::vector<uint32_t> permutation(nodes.size());
        for(auto i = 0; i < nodes.size(); i+=1)
            permutation[i] = i;
        std::random_shuffle(permutation.begin(), permutation.end());

        // group in blocks of size K, if size not divisible by K, last group has size % K members
        for(auto i = 0; i<nodes.size(); i+=K) {
            for(auto e = 0; e<K && i+e<nodes.size(); e+=1) {
                for(auto add = e+1; add<K && i+add<nodes.size(); add+=1){
                    nodes[permutation[i+e]].addConnection(nodes[permutation[i+add]],tag);
                }
            }
        }

    }
};

#endif //NETSIM2_NET_CREATION_H
