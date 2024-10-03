#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <sstream>
#include <set>
#include <iostream>
#include <stdexcept>
#include <climits>

class Graph{
public:
    Graph(int vertices);  // Constructor
    ~Graph();  // Destructor
    
    // Origin Graph Functions
    void addEdge(int u, int v, int weight);  // Add edge to graph
    void removeEdge(int u, int v);  // Remove edge from graph
    int getEdgeWeight(int u, int v) const;  // Get weight of edge
    int getSizeVertices() const;  // Get number of vertices
    const std::vector<std::vector<int>> &getGraph() const;  // Get adjacency matrix
    
    // MST Graph Functions - after activate MST Algorithm
    void setMST(std::vector<std::vector<int>>* mst);  // Set adjacency matrix - MST representations
    int getMSTTotalWeight() const;
    int getMSTLongestDistance() const;
    int getMSTShortestDistance() const;
    double getMSTAvgEdgeWeight() const;
    std::string printMST() const;

private:
    int numVertices;
    int totalWeightMST;
    std::vector<std::vector<int>> adjMatrix;  // Adjacency matrix - graph representations
    std::vector<std::vector<int>>* mstMatrix;  // Adjacency matrix - MST representations

};

#endif
