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
    serverObj->start();
    

    // Create the graph using new
    Graph* graph = new Graph(7);
    graph->addEdge(0, 1, 2);
    graph->addEdge(0, 2, 3);
    graph->addEdge(0, 5, 4);
    graph->addEdge(1, 3, 2);
    graph->addEdge(1, 4, 4);
    graph->addEdge(2, 5, 5);
    graph->addEdge(3, 6, 5);

    std::cout<< "Prim Algorithm:"<<std::endl;
    // Use Prim's algorithm to compute the MST
    MSTStrategy* strategy = MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Prim);
    graph->setMST(strategy->computeMST(*graph));
    graph->printMST();
    std::cout<<"MST Total Weight: "<<graph->getMSTTotalWeight()<<std::endl;
    std::cout<<"MST Average Distance: "<<graph->getMSTAvgEdgeWeight()<<std::endl;
    std::cout<<"MST Longest Distance: "<<graph->getMSTLongestDistance()<<std::endl;
    std::cout<<"MST Shortest Distance: "<<graph->getMSTShortestDistance()<<std::endl;
    std::cout<<std::endl;
    delete strategy;

    std::cout<< "Kruskal Algorithm:"<<std::endl;
    // Use Kruskal's algorithm to compute the MST
    strategy = MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Kruskal);
    graph->setMST(strategy->computeMST(*graph));
    graph->printMST();
    std::cout<<"MST Total Weight: "<<graph->getMSTTotalWeight()<<std::endl;
    std::cout<<"MST Average Distance: "<<graph->getMSTAvgEdgeWeight()<<std::endl;
    std::cout<<"MST Longest Distance: "<<graph->getMSTLongestDistance()<<std::endl;
    std::cout<<"MST Shortest Distance: "<<graph->getMSTShortestDistance()<<std::endl;
    std::cout<<std::endl;

    // Modify the graph
    graph->removeEdge(3, 6);

    std::cout<< "Kruskal Algorithm - after changes:"<<std::endl;
    // Use Kruskal's algorithm to compute the MST
    strategy = MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Kruskal);
    graph->setMST(strategy->computeMST(*graph));
    graph->printMST();
    std::cout<<"MST Total Weight: "<<graph->getMSTTotalWeight()<<std::endl;
    std::cout<<"MST Average Distance: "<<graph->getMSTAvgEdgeWeight()<<std::endl;
    std::cout<<"MST Longest Distance: "<<graph->getMSTLongestDistance()<<std::endl;
    std::cout<<"MST Shortest Distance: "<<graph->getMSTShortestDistance()<<std::endl;


    std::cout<<std::endl;

    delete strategy;
    std::cout<< "Prim Algorithm - after changes:"<<std::endl;
    // Use Prim's algorithm to compute the MST
    strategy = MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Prim);
    graph->setMST(strategy->computeMST(*graph));
    graph->printMST();
    std::cout<<"MST Total Weight: "<<graph->getMSTTotalWeight()<<std::endl;
    std::cout<<"MST Average Distance: "<<graph->getMSTAvgEdgeWeight()<<std::endl;
    std::cout<<"MST Longest Distance: "<<graph->getMSTLongestDistance()<<std::endl;
    std::cout<<"MST Shortest Distance: "<<graph->getMSTShortestDistance()<<std::endl;

    // Clean up the allocated memory
    delete graph;
    delete strategy;
    delete serverObj;

    return 0;
}