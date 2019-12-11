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
uint32_t global_concount = 5;
double global_p = 0.1;

// if some protocol specific initialization is required, use this
template<typename T>
void protInit() {}
template<>
void protInit<experiments::nodeAD>() {
    experiments::nodeAD::d = global_d;
    experiments::nodeAD::D = global_concount;
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

template <typename T>
std::string fileName(uint32_t nodecount, const std::vector<uint32_t>& concounts) {
    return std::to_string(nodecount)+"_k_"+std::to_string(concounts[0]);
}
template <>
std::string fileName<experiments::node3pp>(uint32_t nodecount, const std::vector<uint32_t>& concounts) {
    return protName<experiments::node3pp>()+"_"+fileName<int>(nodecount,concounts)
           +"_g_"+std::to_string(concounts[1])
           +"_a_"+std::to_string(global_d_3pp)
           +"_N_"+std::to_string(global_N_3pp);
}
template <>
std::string fileName<experiments::nodefap>(uint32_t nodecount, const std::vector<uint32_t>& concounts) {
    return protName<experiments::nodefap>()+"_"+fileName<int>(nodecount,concounts);
}
template <>
std::string fileName<experiments::nodeAD>(uint32_t nodecount, const std::vector<uint32_t>& concounts) {
    return protName<experiments::nodeAD>()+"_"+fileName<int>(nodecount,concounts)
                                                +"_d_"+std::to_string(global_d);
}
template <>
std::string fileName<experiments::nodedd>(uint32_t nodecount, const std::vector<uint32_t>& concounts) {
    return protName<experiments::nodedd>()+"_"+fileName<int>(nodecount,concounts)
                                                +"_p_"+std::to_string(global_p);
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
    const std::string filename = fileName<T>(nodecount,concounts)+".csv";
    /*std::ofstream file;
    file.open (filename,std::ofstream::out | std::ofstream::trunc);
    file.close();
    file.open(filename,std::ios::app);
    */

    // Protocoll specific tasks:
    protInit<T>();
    //network<T> net(std::cout, file, getStrategies<T>(concounts), nodecount, constModel<10>);
    network<T> net(false, getStrategies<T>(concounts), nodecount, normalModel<80,200>(20,1));

    net.startProtocolOn(starter);

    net.runSimulation();

    // finally
    //file.close();

    std::ofstream pr_log;

    pr_log.open("progress_"+filename,std::ios::app);
    net.writeProgressProtocol(pr_log);
    pr_log.close();

    auto hasntSeen = 0;
    for(auto& node : net.getNodes()) {
        if(!node.hasSeen(0)){
            hasntSeen+=1;
        }
    }

    return std::make_tuple(hasntSeen,net.sim.now());
}

template<>
std::tuple<uint64_t,uint64_t> runExperiment<experiments::node3pp>(uint32_t nodecount, std::vector<nodeid> starter, std::vector<uint32_t> concounts) {
    const std::string filename = fileName<experiments::node3pp>(nodecount, concounts)+".csv";
            /*std::ofstream file;
    file.open (filename,std::ofstream::out | std::ofstream::trunc);
    file.close();
    file.open(filename,std::ios::app);
    */

    // Protocoll specific tasks:
    protInit<experiments::node3pp>();
    //network<experiments::node3pp> net(std::cout, file, getStrategies<experiments::node3pp>(concounts), nodecount, constModel<10>);
    network<experiments::node3pp> net(false, getStrategies<experiments::node3pp>(concounts), nodecount, normalModel<80,200>(20,1));

    if(concounts.size()>1){
        std::minstd_rand gen(std::random_device{}());
        std::uniform_int_distribution<> dist(0,nodecount-1);
        for(auto& s : starter){
            while(net.getNodes()[s].amountConnections(experiments::pp::networktag::group)!=concounts[1]-1){
                s = dist(gen);
            }
        }
    }
    net.startProtocolOn(starter);

    net.runSimulation();

    // finally
    //file.close();

    std::ofstream ph_log,pr_log;
    ph_log.open("phaselog_"+filename,std::ios::app);
    net.writePhaseProtocol(ph_log);
    ph_log.close();

    pr_log.open("progress_"+filename,std::ios::app);
    net.writeProgressProtocol(pr_log);
    pr_log.close();

    auto hasntSeen = 0;
    for(auto& node : net.getNodes()) {
        if(!node.hasSeen(0)){
            hasntSeen+=1;
        }
    }

    return std::make_tuple(hasntSeen,net.sim.now());
}
#endif //NETSIM2_RUN_H
