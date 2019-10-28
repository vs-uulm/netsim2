//
// Created by moedinger on 25.10.19.
//

#include <iostream>
#include <fstream>

#include "experiments/run.h"

int main(int argc, char *argv[]) {
    // parse input to form parameters
    uint32_t nodecount = 10000;
    uint32_t concount = 5;

    if (argc == 2) {
        if (argv[1][1] == 'h') {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "\t-n<number> number of participants, default: " << nodecount << std::endl;
            std::cout << "\t-c<number> number of connections per node, default: " << concount << std::endl;
            std::cout << "\t-p<number> Dandelion probabilty as per mille, default: " << global_p << "(=> p/1000)" << std::endl;
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
                case 'c':
                    concount = stoi(param.substr(2));
                    break;
                case 'p':
                    global_p = stoi(param.substr(2))/1000.0;
                    break;
            }
        }
    }

    std::ofstream file;
    std::string filename = "DD_simresults.csv";
    std::ifstream output(filename);
    bool fileexisted = output.good();
    file.open(filename, std::ios::app);

    // write csv header if file didnt exist before
    if(!fileexisted) {
        file << "nodecount, conection count, starting node,Dandelion probability,unreached nodes, simtime";
        file << std::endl;
    }

    // generate a random starter node
    std::minstd_rand gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0,nodecount-1);
    std::vector<nodeid> starters;
    starters.push_back(dist(gen));

    std::vector<uint32_t> concounts{concount};

    auto start = time(NULL);

    file << nodecount << "," << concount << "," << starters[0] << "," << global_p;

    auto result = runExperiment<experiments::nodedd>(nodecount, starters, concounts);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);
    file << std::endl;

    auto end = time(NULL)-start;
    std::cout << time(NULL) << " " << protName<experiments::nodedd>() << ": It took " << end << "s. Missrate: " << std::get<0>(result)*100.0/nodecount << "%" << std::endl;

    return 0;
}