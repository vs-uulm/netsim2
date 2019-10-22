#include <iostream>

#include "experiments/nodeAD.h"

int main(int argc, char *argv[]) {
    // parse input to form parameters
    uint32_t nodecount = 10000;
    uint32_t d = 5;
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

    experiments::runSimulationAD(nodecount,d);

    return 0;
}