//
// Created by moedinger on 25.10.19.
//

#include <fstream>
#include <iostream>

#include "experiments/run.h"

int main(int argc, char *argv[]) {
    // parse input to form parameters
    uint32_t nodecount = 100;
    uint32_t concount = 8;

    if (argc == 2) {
        if (argv[1][1] == 'h') {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "\t-n<number> number of participants, default: " << nodecount << std::endl;
            std::cout << "\t-c<number> number of connections per node, default: " << concount << std::endl;
            std::cout << "\t-d<number> AD depth, default: " << global_d << std::endl;
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
                case 'd':
                    global_d = stoi(param.substr(2));
                    break;
            }
        }
    }

    global_d = 6;

    std::ofstream file;
    std::string filename = "AD_simresults.csv";
    file.open(filename, std::ios::app);

    // generate a random starter node
    std::minstd_rand gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0,nodecount-1);
    std::vector<nodeid> starters;
    starters.push_back(dist(gen));

    global_concount = concount;
    std::vector<uint32_t> concounts{concount};

    auto start = time(NULL);

    file << nodecount << "," << concount << "," << starters[0] << "," << global_d;
    auto result = runExperiment<experiments::nodeAD>(nodecount, starters, concounts);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);
    file << std::endl;

    auto end = time(NULL)-start;
    std::cout << time(NULL) << " " << protName<experiments::nodeAD>() << ": It took " << end << "s. Missrate: " << std::get<0>(result)*100.0/nodecount << "%" << std::endl;

    return 0;
}