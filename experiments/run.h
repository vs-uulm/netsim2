//
// Created by moedinger on 24.10.19.
//

#ifndef NETSIM2_RUN_H
#define NETSIM2_RUN_H

#include <cstdint>
#include <fstream>
#include "AD/nodeAD.h"
#include "DD/nodedd.h"
#include "3PP/node3pp.h"
#include "FAP/nodefap.h"
#include "../netsim_basic/net_creation.h"
#include "../netsim_basic/net_edge_models.h"

// bad solution to mulitple parameters passing to the templated functions
// so there is only one api I can call
uint32_t global_d = 5;
uint32_t global_d_3pp = 5;
uint32_t global_N_3pp = 5;
double global_p = 0.1;

// if some protocol specific initialization is required, use this
template<typename T>
void protInit() {}
template<>
void protInit<experiments::nodeAD>() {
    experiments::nodeAD::d = global_d;
}
template<>
void protInit<experiments::nodedd>() {
    experiments::nodedd::p_phasechange = global_p;
}
template<>
void protInit<experiments::node3pp>() {
    experiments::node3pp::d = global_d_3pp;
    experiments::node3pp::N = global_N_3pp;
}

template<typename T>
std::string protName(){
    return "None";
};
template<>
std::string protName<experiments::nodeAD>() {
    return "AD";
}
template<>
std::string protName<experiments::nodedd>() {
    return "DD";
}
template<>
std::string protName<experiments::nodefap>() {
    return "FP";
}
template<>
std::string protName<experiments::node3pp>() {
    return "3P";
}

template<typename T>
std::vector<std::function<void(std::vector<T>&)>> getStrategies(const std::vector<uint32_t>& concounts) {
    std::vector<std::function<void(std::vector<T>&)>> strategies{uniformlyAtLeastK<T>(concounts[0])};
    return strategies;
}
template<>
std::vector<std::function<void(std::vector<experiments::node3pp>&)>> getStrategies(const std::vector<uint32_t>& concounts) {
    std::vector<std::function<void(std::vector<experiments::node3pp> &)>> strategies{
        uniformlyAtLeastK<experiments::node3pp>(concounts[0],experiments::pp::networktag::broadcast),
                groupOfSizeK<experiments::node3pp>(concounts[1],experiments::pp::networktag::group)
                };
    return strategies;
}

template<typename T>
std::tuple<uint64_t,uint64_t> runExperiment(uint32_t nodecount, std::vector<nodeid> starter, std::vector<uint32_t> concounts) {
    const std::string filename = std::to_string(nodecount)+"_k-"+std::to_string(concounts[0])+(concounts.size()>1?"_"+std::to_string(concounts[1]):"")+"_"+protName<T>()+".csv";
    /*std::ofstream file;
    file.open (filename,std::ofstream::out | std::ofstream::trunc);
    file.close();
    file.open(filename,std::ios::app);
    */
    //std::ofstream devnull("/dev/null");

    // Protocoll specific tasks:
    protInit<T>();
    //network<T> net(std::cout, file, getStrategies<T>(concounts), nodecount, constModel<10>);
    //network<T> net(std::cout, devnull, getStrategies<T>(concounts), nodecount, constModel<10>);
    network<T> net(false, getStrategies<T>(concounts), nodecount, constModel<10>);

    net.startProtocolOn(starter);

    net.runSimulation();

    // finally
    //file.close();

    auto hasntSeen = 0;
    for(auto& node : net.getNodes()) {
        if(!node.hasSeen(0)){
            hasntSeen+=1;
        }
    }
    std::cout << protName<T>() << ": ";
    std::cout << nodecount-hasntSeen << " (" << (nodecount-hasntSeen)*100.0/nodecount;
    std::cout << "%) of nodes did receive the message. Missing " << hasntSeen << " nodes.";
    std::cout << " In " << net.sim.now() << "ms." << std::endl;

    return std::make_tuple(hasntSeen,net.sim.now());
}
#endif //NETSIM2_RUN_H
