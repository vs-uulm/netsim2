#include <iostream>
#include <fstream>

#include "experiments/run.h"

int main(int argc, char *argv[]) {
    // parse input to form parameters
    uint32_t nodecount = 10000;
    uint32_t concount = 5;
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
    auto result = runExperiment<experiments::nodeAD>(nodecount, starters, concount);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);

    result = runExperiment<experiments::nodefap>(nodecount, starters, concount);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);

    result = runExperiment<experiments::nodedd>(nodecount, starters, concount);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);

    result = runExperiment<experiments::node3pp>(nodecount, starters, concount);
    file << "," << std::get<0>(result) << "," << std::get<1>(result);
    file << std::endl;

    std::cout << time(NULL) << ": Finished. It took " << time(NULL)-start << "s." << std::endl;

    return 0;
}