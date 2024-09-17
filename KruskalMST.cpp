#include "MSTStrategy.hpp"
#include <vector>
#include <numeric>
#include <algorithm>

class KruskalMST : public MSTStrategy
{
public:
    void computeMST(const Graph &graph) override;
    int getTotalWeight() const override;
    int getLongestDistance() const override;
    double getAverageDistance() const override;
    int getShortestDistance() const override;

private:
    struct Edge
    {
        int u, v, weight;
    };
    static bool compareEdges(const Edge &e1, const Edge &e2)
    {
        return e1.weight < e2.weight;
    }
    int totalWeight;
    std::vector<Edge> mstEdges;
};

void KruskalMST::computeMST(const Graph &graph)
{
    int vertices = graph.getVertices();
    std::vector<Edge> edges;
    totalWeight = 0;

    for (int u = 0; u < vertices; ++u)
    {
        for (int v = u + 1; v < vertices; ++v)
        {
            int weight = graph.getWeight(u, v);
            if (weight)
                edges.push_back({u, v, weight});
        }
    }

    std::sort(edges.begin(), edges.end(), compareEdges);

    std::vector<int> parent(vertices);
    std::iota(parent.begin(), parent.end(), 0);

    auto find = [&](int u)
    {
        while (u != parent[u])
            u = parent[u];
        return u;
    };

    auto unite = [&](int u, int v)
    {
        parent[find(u)] = find(v);
    };

    for (const auto &edge : edges)
    {
        if (find(edge.u) != find(edge.v))
        {
            unite(edge.u, edge.v);
            mstEdges.push_back(edge);
            totalWeight += edge.weight;
        }
    }
}

int KruskalMST::getTotalWeight() const
{
    return totalWeight;
}

int KruskalMST::getLongestDistance() const
{
    // Placeholder implementation
    return 0;
}

double KruskalMST::getAverageDistance() const
{
    // Placeholder implementation
    return 0.0;
}

int KruskalMST::getShortestDistance() const
{
    // Placeholder implementation
    return 0;
}
