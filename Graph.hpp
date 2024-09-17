#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>

class Graph
{
public:
    Graph(int vertices);
    void addEdge(int u, int v, int weight);
    void removeEdge(int u, int v);
    int getWeight(int u, int v) const;
    int getVertices() const;
    const std::vector<std::vector<int>> &getAdjMatrix() const;

private:
    int vertices;
    std::vector<std::vector<int>> adjMatrix;
};

#endif
