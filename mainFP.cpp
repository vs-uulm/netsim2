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
            std::cout << "Usage: " << argv[0] << " -n<number> -t<number> -r<number>" << std::endl;
            std::cout << "\t-n<number> number of participants, default: " << nodecount << std::endl;
            std::cout << "\t-c<number> number of connections per node, default: " << concount << std::endl;
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
            }
        }
    }

    std::ofstream file;
    std::string filename = "FP_simresults.csv";
    std::ifstream output(filename);
    bool fileexisted = output.good();
    file.open(filename, std::ios::app);

    // write csv header if file didnt exist before
    if(!fileexisted) {
        file << "nodecount, conection count, groupsize,starting node,unreached nodes, simtime";
        file << std::endl;
    }

    // generate a random starter node
    std::minstd_rand gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0,nodecount-1);
    std::vector<nodeid> starters;
    starters.push_back(dist(gen));

    std::vector<uint32_t> concounts{concount};

    auto start = time(NULL);

    file << nodecount << "," << concount << "," << starters[0];

    auto result = runExperiment<experiments::nodefap>(nodecount, starters, concounts);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);

    auto end = time(NULL)-start;
    std::cout << time(NULL) << ": Finished. It took " << end << "s." << std::endl;

    return 0;
}