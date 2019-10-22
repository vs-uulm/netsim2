#include <iostream>
#include <fstream>

#include "experiments/AD/nodeAD.h"
#include "experiments/DD/nodedd.h"
#include "experiments/FAP/nodefap.h"
#include "netsim_basic/net_creation.h"
#include "netsim_basic/net_edge_models.h"


// i made the mistake to have the strategies use templates
// so i need it as a const parameter... but to only have it once, it's here
// don't change it to dynamic.
const auto concount = 5;

// if some protocol specific initialization is required, use this
template<typename T>
void protInit(uint32_t prot_spec) {}
template<>
void protInit<experiments::nodeAD>(uint32_t prot_spec) {
    experiments::nodeAD::d = prot_spec;
}
template<>
void protInit<experiments::nodedd>(uint32_t prot_spec) {
    experiments::nodedd::p_phasechange = prot_spec/100.0;
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
//template<>
//std::string protName<experiments::node3pp>() {
//    return "3pp";
//}

template<typename T>
void runExperiment(uint32_t nodecount, uint32_t prot_spec) {
    const std::string filename = std::to_string(nodecount)+"_k-"+std::to_string(concount)+"_"+protName<T>()+".csv";
    std::ofstream file;
    file.open (filename,std::ofstream::out | std::ofstream::trunc);
    file.close();
    file.open(filename,std::ios::app);

    // Protocoll specific tasks:
    protInit<T>(prot_spec);
    std::vector<std::function<void(std::vector<T>&)>> strategies{uniformlyAtLeastK<T,concount>};
    network<T> net(std::cout, file, strategies, nodecount, constModel<10>);

    net.startProtocolOn({1});

    net.runSimulation();

    // finally
    file.close();

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
}

int main(int argc, char *argv[]) {
    // parse input to form parameters
    uint32_t nodecount = 10000;
    uint32_t d = 6;
    bool verbosity = false;

    if (argc == 2) {
        if (argv[1][1] == 'h') {
            std::cout << "Usage: " << argv[0] << " -n<number> -t<number> -r<number>" << std::endl;
            std::cout << "\t-n<number> number of participants, default: " << nodecount << std::endl;
            std::cout << "\t-d<number> protocol dependent parameter, default: " << d << std::endl;
            std::cout << "\t-v implies verbosity" << std::endl;
            return 0;
        }
    }

    for (auto c = 1; c < argc; c++) {
        std::string param(argv[c]);
        if (param[0] == '-') {
            switch (param[1]) {
                case 'n':
                    nodecount = stoi(param.substr(2));
                    break;
                case 'd':
                    d = stoi(param.substr(2));
                    break;
                case 'v':
                    verbosity = true;
                    break;
            }
        }
    }

    auto start = time(NULL);

    runExperiment<experiments::nodeAD>(nodecount, d);
    runExperiment<experiments::nodefap>(nodecount, d);
    runExperiment<experiments::nodedd>(nodecount, d);
    //runExperiment<experiments::node3pp>(nodecount, d);

    std::cout << time(NULL) << ": Finished. It took " << time(NULL)-start << "s." << std::endl;

    return 0;
}