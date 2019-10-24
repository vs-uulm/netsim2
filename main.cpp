#include <iostream>
#include <fstream>

#include "experiments/AD/nodeAD.h"
#include "experiments/DD/nodedd.h"
#include "experiments/FAP/nodefap.h"
#include "netsim_basic/net_creation.h"
#include "netsim_basic/net_edge_models.h"
#include "experiments/3PP/node3pp.h"


// i made the mistake to have the strategies use templates
// so i need it as a const parameter... but to only have it once, it's here
// don't change it to dynamic.
const auto concount = 5;


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
std::tuple<uint64_t,uint64_t> runExperiment(uint32_t nodecount, std::vector<nodeid> starter) {
    const std::string filename = std::to_string(nodecount)+"_k-"+std::to_string(concount)+"_"+protName<T>()+".csv";
    std::ofstream file;
    file.open (filename,std::ofstream::out | std::ofstream::trunc);
    file.close();
    file.open(filename,std::ios::app);

    // Protocoll specific tasks:
    protInit<T>();
    std::vector<std::function<void(std::vector<T>&)>> strategies{uniformlyAtLeastK<T>(concount)};
    network<T> net(std::cout, file, strategies, nodecount, constModel<10>);

    net.startProtocolOn(starter);

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

    return std::make_tuple(hasntSeen,net.sim.now());
}

int main(int argc, char *argv[]) {
    // parse input to form parameters
    uint32_t nodecount = 10000;
    bool verbosity = false;

    if (argc == 2) {
        if (argv[1][1] == 'h') {
            std::cout << "Usage: " << argv[0] << " -n<number> -t<number> -r<number>" << std::endl;
            std::cout << "\t-n<number> number of participants, default: " << nodecount << std::endl;
            std::cout << "\t-d<number> AD depth, default: " << global_d << std::endl;
            std::cout << "\t-a<number> 3pp-AD depth, default: " << global_d_3pp << std::endl;
            std::cout << "\t-N<number> 3pp-AD-N, default: " << global_N_3pp << std::endl;
            std::cout << "\t-p<number> Dandelion probabilty as per mille, default: " << global_p << "(=> p/1000)" << std::endl;
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
                    global_d = stoi(param.substr(2));
                    break;
                case 'p':
                    global_p = stoi(param.substr(2))/1000.0;
                    break;
                case 'a':
                    global_d_3pp = stoi(param.substr(2))/1000.0;
                    break;
                case 'N':
                    global_N_3pp  = stoi(param.substr(2))/1000.0;
                    break;
                case 'v':
                    verbosity = true;
                    break;
            }
        }
    }

    std::ofstream file;
    file.open("simresults.csv", std::ios::app);

    // generate a random starter node
    std::minstd_rand gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0,nodecount-1);
    std::vector<nodeid> starters;
    starters.push_back(dist(gen));

    auto start = time(NULL);

    file << nodecount << "," << starters[0] << ",";
    file << global_d << "," << global_p << "," << global_d_3pp << "," << global_N_3pp;
    auto result = runExperiment<experiments::nodeAD>(nodecount, starters);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);

    result = runExperiment<experiments::nodefap>(nodecount, starters);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);

    result = runExperiment<experiments::nodedd>(nodecount, starters);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);

    result = runExperiment<experiments::node3pp>(nodecount, starters);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);
    file << std::endl;

    std::cout << time(NULL) << ": Finished. It took " << time(NULL)-start << "s." << std::endl;

    return 0;
}