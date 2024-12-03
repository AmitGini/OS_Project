#include "KruskalStrategy.hpp"

// Helper function to perform DFS to check for cycles
bool KruskalStrategy::hasCycle(int current, int parent, const std::vector<std::vector<int>> &adj, std::vector<bool> &visited)
{
    visited[current] = true;
    int numVer = adj.size(); // Number of vertices in the graph
    for (int neighbor = 0; neighbor < numVer; ++neighbor)
    {
        if (adj[current][neighbor] != 0)
        {
            if (!visited[neighbor])
            {
                if (hasCycle(neighbor, current, adj, visited))
                    return true;
            }
            else if (neighbor != parent)
            {
                return true;
            }
        }
    }
    return false;
}

std::unique_ptr<std::vector<std::vector<int>>> KruskalStrategy::computeMST(const std::vector<std::vector<int>> &graphAdjacencyMatrix)
{
    {
        std::lock_guard<std::mutex> cout_lock(cout_mtx);
        std::cout << "Strategy Activated - Start Compute MST using Kruskal" << std::endl;
    }
    int numVertices = graphAdjacencyMatrix.size();
    std::vector<std::tuple<int, int, int>> edges; // (weight, src, dest) To keep track of included edges for sorting and processing

    // Collect all edges from the adjacency matrix
    for (int i = 0; i < numVertices; i++)
    {
        for (int j = i + 1; j < numVertices; j++)
        {
            if (graphAdjacencyMatrix[i][j] > 0)
            {
                edges.emplace_back(graphAdjacencyMatrix[i][j], i, j);
            }
        }
    }

    auto mstMatrix = std::make_unique<std::vector<std::vector<int>>>(
        numVertices, std::vector<int>(numVertices, 0));

    std::sort(edges.begin(), edges.end()); // Sort edges by their weights
    int edgesAdded = 0;

    // Kruskal's algorithm - Adding edges to the MST, checking for cycles
    for (const auto &eg : edges)
    {
        int weight = std::get<0>(eg);
        int u = std::get<1>(eg);
        int v = std::get<2>(eg);
        (*mstMatrix)[v][u] = weight;
        (*mstMatrix)[u][v] = weight;

        // Check for a cycle using DFS
        std::vector<bool> visited(numVertices, false);
        if (hasCycle(u, -1, *mstMatrix, visited))
        {
            // If adding this edge creates a cycle, remove it
            (*mstMatrix)[u][v] = 0;
            (*mstMatrix)[v][u] = 0;
        }
        else
        {
            // If no cycle is formed, continue
            edgesAdded++;
            if (edgesAdded == numVertices - 1)
                break; // Stop when enough edges have been added
        }
    }
    {
        std::lock_guard<std::mutex> cout_lock(cout_mtx);
        std::cout << "Finish Compute MST using Kruskal" << std::endl;
    }
    // Reference of the MST adjacency matrix
    return mstMatrix;
}
