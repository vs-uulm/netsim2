//
// Created by moedinger on 16.10.19.
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

#endif //NETSIM2_NET_EDGE_MODELS_H
