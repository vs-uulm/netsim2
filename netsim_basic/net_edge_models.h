//
// Created by moedinger on 16.10.19.
// https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=832574
// https://dl.acm.org/citation.cfm?id=637203
//

#ifndef NETSIM2_NET_EDGE_MODELS_H
#define NETSIM2_NET_EDGE_MODELS_H

#include <cstdint>

/**
 * Provides a constant function of the signature required for network edge models double(void)
 */
template<uint32_t d>
double constModel() {
    return d;
}

template<uint32_t gamma = 0, uint32_t max = 500>
class normalModel {
private:
    std::default_random_engine gen;
    std::normal_distribution<double> dist;

public:
    normalModel(double mu, double si) : dist(mu, si) {}

    double operator()() {
        return std::max(std::min(double(dist(gen)),double(max)), double(gamma));
    }
};

template<uint32_t gamma = 0, uint32_t max = 500>
class lognormalModel {
private:
    std::default_random_engine gen;
    std::lognormal_distribution<double> dist;

public:
    lognormalModel(double mu, double si) : dist(mu, si) {}

    double operator()() {
        return std::min(double(dist(gen))+gamma, double(max));
    }
};

#endif //NETSIM2_NET_EDGE_MODELS_H
