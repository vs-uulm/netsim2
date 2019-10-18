cmake_minimum_required(VERSION 3.15)
project(netsim2)

set(CMAKE_CXX_STANDARD 17)

add_executable(netsim2 main.cpp simulator.cpp simulator.h netsim_basic/net_edge_models.h netsim_basic/net_creation.h netsim_basic/net_nodes.h net_network.h examples/basic_example.h experiments/node3pp.cpp experiments/node3pp.h experiments/nodefap.cpp experiments/nodefap.h)