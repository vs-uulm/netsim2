#include <iostream>

#include "experiments/nodefap.h"

int main(int argc, char *argv[]) {
    // parse input to form parameters
    uint32_t nodecount = 30;
    uint32_t txcount = 1;
    uint32_t simlength = 1000 * 60 * 60;
    bool verbosity = false;

    if (argc == 2) {
        if (argv[1][1] == 'h') {
            std::cout << "Usage: " << argv[0] << " -n<number> -t<number> -r<number>" << std::endl;
            std::cout << "\t-n<number> number of participants, default: " << nodecount << std::endl;
            std::cout << "\t-t<number> number of transactions, default: " << txcount << std::endl;
            std::cout << "\t-r<number> duratino of simulation in ms, default: " << simlength << std::endl;
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
                case 't':
                    txcount = stoi(param.substr(2));
                    break;
                case 'r':
                    simlength = stoi(param.substr(2));
                    break;
                case 'v':
                    verbosity = true;
                    break;
            }
        }
    }

    //examples::runSimulation();
    experiments::runSimulationFAP();

    return 0;
}