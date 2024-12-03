#include "PrimStrategy.hpp"
#include <iostream>
#include <algorithm>

std::unique_ptr<std::vector<std::vector<int>>> PrimStrategy::computeMST(const std::vector<std::vector<int>> &graphAdjacencyMatrix)
{
    {
        std::lock_guard<std::mutex> cout_lock(cout_mtx);
        std::cout << "Strategy Activated - Start Compute MST using Prim" << std::endl;
    }
    int numVertices = graphAdjacencyMatrix.size();
    
    if (numVertices == 0)
        return nullptr;

    std::vector<int> minEdgeToVertex(numVertices, std::numeric_limits<int>::max());
    std::vector<int> parentVertex(numVertices, -1);
    std::vector<bool> isInMST(numVertices, false);
    std::priority_queue<std::pair<int, int>,
                        std::vector<std::pair<int, int>>,
                        std::greater<std::pair<int, int>>>
        minEdgeQueue;

    // Find the first non-isolated vertex
    int startVertex = 0;
    for (int i = 0; i < numVertices; ++i)
    {
        if (std::any_of(graphAdjacencyMatrix[i].begin(), graphAdjacencyMatrix[i].end(), [](int w)
                        { return w != 0; }))
        {
            startVertex = i;
            break;
        }
    }

    

    minEdgeToVertex[startVertex] = 0;
    minEdgeQueue.push({0, startVertex});

    while (!minEdgeQueue.empty())
    {
        int currentVertex = minEdgeQueue.top().second;
        minEdgeQueue.pop();

        if (isInMST[currentVertex])
            continue;

        isInMST[currentVertex] = true;

        for (int adjacentVertex = 0; adjacentVertex < numVertices; ++adjacentVertex)
        {
            int weight = graphAdjacencyMatrix[currentVertex][adjacentVertex];
            if (weight != 0 && !isInMST[adjacentVertex] && weight < minEdgeToVertex[adjacentVertex])
            {
                parentVertex[adjacentVertex] = currentVertex;
                minEdgeToVertex[adjacentVertex] = weight;
                minEdgeQueue.push({weight, adjacentVertex});
            }
        }
    }

    auto mstMatrix = std::make_unique<std::vector<std::vector<int>>>(
        numVertices, std::vector<int>(numVertices, 0));

    for (int vertex = 0; vertex < numVertices; ++vertex)
    {
        int parent = parentVertex[vertex];
        if (parent != -1)
        {
            int weight = graphAdjacencyMatrix[parent][vertex];
            (*mstMatrix)[parent][vertex] = weight;
            (*mstMatrix)[vertex][parent] = weight;
        }
    }

    std::lock_guard<std::mutex> cout_lock(cout_mtx);
    std::cout << "Finish Compute MST using Prim" << std::endl;
    return mstMatrix;
}