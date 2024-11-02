#include <iostream>
#include <unistd.h>
#include "Graph.hpp"
#include "MSTFactory.hpp"
#include "MSTStrategy.hpp"
#include "PrimStrategy.hpp"
#include "KruskalStrategy.hpp"
#include "Server.hpp"

int main(int argc, char* argv[]) {
    bool pipelineFlag = false, leaderFollowerFlag = false;

    // Parsing command-line arguments
    int opt = 0;
    while ((opt = getopt(argc, argv, "pl")) != -1) {
        switch (opt) {
            case 'p':
                pipelineFlag = true;
                break;
            case 'l':
                leaderFollowerFlag = true;
                break;
            default:
                std::cerr << "Usage: ./" << argv[0] << " [-p] [-l]" << std::endl
                          << "Example: " << argv[0] << " -p (for Pipeline)" << std::endl
                          << "         " << argv[0] << " -l (for LeaderFollower)" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    if((!pipelineFlag && !leaderFollowerFlag) || (pipelineFlag && leaderFollowerFlag)) {
        std::cerr << "Must choose 1 type of design pattern (-p:Pipeline -l:LeaderFollower)" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    Server* serverObj = pipelineFlag ? new Server(true) : new Server(false);
    delete serverObj;

    return 0;
}