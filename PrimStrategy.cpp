#include "PrimStrategy.hpp"
#include <iostream>
#include <algorithm>

std::vector<std::vector<int>>* PrimStrategy::computeMST(const Graph &graph) {
    int numVertices = graph.getSizeVertices();
    std::cout << "Number of vertices: " << numVertices << std::endl;
    
    if (numVertices == 0) return nullptr;
    
    const auto& adjacencyMatrix = graph.getGraph();
    
    // Debug: Print adjacency matrix
    std::cout << "Adjacency Matrix:" << std::endl;
    for (const auto& row : adjacencyMatrix) {
        for (int weight : row) {
            std::cout << weight << " ";
        }
        std::cout << std::endl;
    }
    
    std::vector<int> minEdgeToVertex(numVertices, std::numeric_limits<int>::max());
    std::vector<int> parentVertex(numVertices, -1);
    std::vector<bool> isInMST(numVertices, false);
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> minEdgeQueue;
    
    // Find the first non-isolated vertex
    int startVertex = 0;
    for (int i = 0; i < numVertices; ++i) {
        if (std::any_of(adjacencyMatrix[i].begin(), adjacencyMatrix[i].end(), [](int w) { return w != 0; })) {
            startVertex = i;
            break;
        }
    }
    
    std::cout << "Starting from vertex: " << startVertex << std::endl;
    
    minEdgeToVertex[startVertex] = 0;
    minEdgeQueue.push({0, startVertex});
    
    while (!minEdgeQueue.empty()) {
        int currentVertex = minEdgeQueue.top().second;
        minEdgeQueue.pop();
        
        if (isInMST[currentVertex]) continue;
        
        isInMST[currentVertex] = true;
        
        std::cout << "Processing vertex: " << currentVertex << std::endl;
        
        for (int adjacentVertex = 0; adjacentVertex < numVertices; ++adjacentVertex) {
            int weight = adjacencyMatrix[currentVertex][adjacentVertex];
            if (weight != 0 && !isInMST[adjacentVertex] && weight < minEdgeToVertex[adjacentVertex]) {
                parentVertex[adjacentVertex] = currentVertex;
                minEdgeToVertex[adjacentVertex] = weight;
                minEdgeQueue.push({weight, adjacentVertex});
                
            }
        }
    }
    
    auto* mstMatrix = new std::vector<std::vector<int>>(numVertices, std::vector<int>(numVertices, 0));
    
    std::cout << "Constructing MST:" << std::endl;
    for (int vertex = 0; vertex < numVertices; ++vertex) {
        int parent = parentVertex[vertex];
        if (parent != -1) {
            int weight = adjacencyMatrix[parent][vertex];
            (*mstMatrix)[parent][vertex] = weight;
            (*mstMatrix)[vertex][parent] = weight;
        }
    }
    

    
    return mstMatrix;
}