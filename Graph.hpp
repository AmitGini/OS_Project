#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <sstream>
#include <stdexcept>
#include <climits>
#include <memory>

class Graph{
private:
    std::vector<std::vector<int>> graphMatrix;  // Adjacency matrix - graph representations
    std::unique_ptr<std::vector<std::vector<int>>> mstMatrix;  // Smart pointer
    int numVertices;
    int numEdges;
    bool hasUpdatedMST;  // Flag to check if MST has been updated

    void validateVertices(int u, int v) const;


public:
    Graph(int vertices);
    ~Graph() = default;  // RAII - Destructor
    
    // Origin Graph Functions
    void addEdge(int u, int v, int weight);  // Add edge to graph
    void removeEdge(int u, int v);  // Remove edge from graph
    int getSizeVertices() const;  // Get number of vertices
    const std::vector<std::vector<int>> &getGraph() const;  // Get adjacency matrix
    bool hasMST() const;  // Check if MST has been computed
    
    // MST Graph Functions - after activate MST Algorithm
    void setMST(std::unique_ptr<std::vector<std::vector<int>>> mst);
    int getMSTTotalWeight() const;
    int getMSTLongestDistance() const;
    int getMSTShortestDistance() const;
    double getMSTAvgEdgeWeight() const;
    std::string printMST() const;


};

#endif
