#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <sstream>
#include <stdexcept>
#include <climits>
#include <memory>
#include <iostream>
#include "MSTStrategy.hpp"

class Graph
{
private:
    std::vector<std::vector<int>> graphMatrix;                // Adjacency matrix - graph representations
    std::unique_ptr<std::vector<std::vector<int>>> mstMatrix; // Smart pointer to the mst matrix
    int numVertices;                                          // Number of vertices in graph
    int numEdges;                                             // Number of edges in graph
    int mstDataStatus;                                        // Flag to check if MST data has been computed
    int mstTotalWeight;                                       // Total weight of MST
    int mstLongestDistance;                                   // Longest distance in MST
    int mstShortestDistance;                                  // Shortest distance in MST
    double mstAvgEdgeWeight;                                  // Average edge weight in MST
    std::unique_ptr<MSTStrategy> mstStrategy;                                 // Pointer to the MST strategy

public:
    Graph(int vertices);
    ~Graph() = default; // RAII - Destructor

    // Origin Graph Functions
    void addEdge(int u, int v, int weight);                // Add edge to graph
    int getSizeVertices() const;                           // Get number of vertices
    const std::vector<std::vector<int>> &getGraph() const; // Get adjacency matrix

    // Setter methods for MST
    void activateMSTStrategy();
    void setMSTDataCalculationNextStatus();
    void setMSTTotalWeight();
    void setMSTLongestDistance();
    void setMSTShortestDistance();
    void setMSTAvgEdgeWeight();
    void setMSTStrategy(std::unique_ptr<MSTStrategy> strategy);

    bool getValidationMSTExist() const;
    int getMSTDataStatusCalculation() const;
    int getMSTTotalWeight() const;
    int getMSTLongestDistance() const;
    int getMSTShortestDistance() const;
    double getMSTAvgEdgeWeight() const;
    std::string printMST() const;
};

#endif
