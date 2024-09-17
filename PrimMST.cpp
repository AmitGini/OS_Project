#include "MSTStrategy.hpp"
#include <vector>
#include <limits>
#include <algorithm>

class PrimMST : public MSTStrategy
{
public:
    void computeMST(const Graph &graph) override;
    int getTotalWeight() const override;
    int getLongestDistance() const override;
    double getAverageDistance() const override;
    int getShortestDistance() const override;

private:
    int totalWeight;
    std::vector<std::vector<int>> mstEdges;
};

void PrimMST::computeMST(const Graph &graph)
{
    int vertices = graph.getVertices();
    std::vector<int> key(vertices, std::numeric_limits<int>::max());
    std::vector<bool> inMST(vertices, false);
    std::vector<int> parent(vertices, -1);

    key[0] = 0;
    totalWeight = 0;
    mstEdges = std::vector<std::vector<int>>(vertices, std::vector<int>(vertices, 0));

    for (int count = 0; count < vertices - 1; ++count)
    {
        int u = -1;
        for (int i = 0; i < vertices; ++i)
            if (!inMST[i] && (u == -1 || key[i] < key[u]))
                u = i;

        inMST[u] = true;

        for (int v = 0; v < vertices; ++v)
        {
            int weight = graph.getWeight(u, v);
            if (weight && !inMST[v] && weight < key[v])
            {
                key[v] = weight;
                parent[v] = u;
            }
        }
    }

    for (int i = 1; i < vertices; ++i)
    {
        mstEdges[parent[i]][i] = graph.getWeight(parent[i], i);
        mstEdges[i][parent[i]] = graph.getWeight(parent[i], i);
        totalWeight += graph.getWeight(parent[i], i);
    }
}

int PrimMST::getTotalWeight() const
{
    return totalWeight;
}

int PrimMST::getLongestDistance() const
{
    // Placeholder implementation
    return 0;
}

double PrimMST::getAverageDistance() const
{
    // Placeholder implementation
    return 0.0;
}

int PrimMST::getShortestDistance() const
{
    // Placeholder implementation
    return 0;
}
