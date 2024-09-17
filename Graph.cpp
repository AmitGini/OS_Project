#include "Graph.hpp"

Graph::Graph(int vertices) : vertices(vertices), adjMatrix(vertices, std::vector<int>(vertices, 0)) {}

void Graph::addEdge(int u, int v, int weight)
{
    adjMatrix[u][v] = weight;
    adjMatrix[v][u] = weight; // Assuming undirected graph
}

void Graph::removeEdge(int u, int v)
{
    adjMatrix[u][v] = 0;
    adjMatrix[v][u] = 0;
}

int Graph::getWeight(int u, int v) const
{
    return adjMatrix[u][v];
}

int Graph::getVertices() const
{
    return vertices;
}

const std::vector<std::vector<int>> &Graph::getAdjMatrix() const
{
    return adjMatrix;
}
