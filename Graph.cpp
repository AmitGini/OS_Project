#include "Graph.hpp"

#define NO_MST_DATA_CALCULATION -1
#define PROGRESS_MST_DATA_CALCULATION 0
#define FINISH_MST_DATA_CALCULATION 1
#define INIT_INTEGER 0
#define INIT_DOUBLE 0.0

Graph::Graph(int vertices)
    : numVertices(vertices), numEdges(INIT_INTEGER),
      mstTotalWeight(INIT_INTEGER), mstLongestDistance(INIT_INTEGER), mstShortestDistance(INT_MAX),
      mstAvgEdgeWeight(INIT_DOUBLE), mstDataStatus(NO_MST_DATA_CALCULATION),
      graphMatrix(vertices, std::vector<int>(vertices, INIT_INTEGER)) {}

// Add edge to graph
void Graph::addEdge(int u, int v, int weight)
{
    if (u < 0 || u >= numVertices || v < 0 || v >= numVertices)
    {
        throw std::out_of_range("Vertex index out of bounds");
    }

    else if (this->graphMatrix[u][v] != 0 || this->graphMatrix[v][u] != 0)
    {
        this->graphMatrix[u][v] = weight;
        this->graphMatrix[v][u] = weight; // Assuming undirected graph
        return;
    }
    else
    {
        this->graphMatrix[u][v] = weight;
        this->graphMatrix[v][u] = weight; // Assuming undirected graph
        this->numEdges++;
    }
}

/*  Getters */

// Get adjacency matrix represent the graph
const std::vector<std::vector<int>> &Graph::getGraph() const
{
    return this->graphMatrix;
}

// Get number of vertices
int Graph::getSizeVertices() const
{
    return this->numVertices;
}

int Graph::getMSTTotalWeight() const
{
    return this->mstTotalWeight;
}

int Graph::getMSTLongestDistance() const
{
    return this->mstLongestDistance;
}

int Graph::getMSTShortestDistance() const
{
    return this->mstShortestDistance;
}

int Graph::getMSTDataStatusCalculation() const
{
    return this->mstDataStatus;
}

double Graph::getMSTAvgEdgeWeight() const
{
    return this->mstAvgEdgeWeight;
}

bool Graph::getValidationMSTExist() const
{
    return this->mstMatrix != nullptr;
}

/*  Setters */

// Set adjacency matrix represent the MST
void Graph::setMST(std::unique_ptr<std::vector<std::vector<int>>> mst)
{
    if (!mst)
    {
        return;
    }
    this->mstMatrix = std::move(mst);
}

// Set the next status of the MST data calculation
void Graph::setMSTDataCalculationNextStatus()
{
    if (this->mstDataStatus == NO_MST_DATA_CALCULATION)
    {
        this->mstDataStatus = PROGRESS_MST_DATA_CALCULATION;
    }
    else if (this->mstDataStatus == PROGRESS_MST_DATA_CALCULATION)
    {
        this->mstDataStatus = FINISH_MST_DATA_CALCULATION;
    }
    else
    {
        std::cout << "** WARNING ** Trying to update MST Data Status - When status is -Finish-" << std::endl;
    }
}

// Calculate and return the total weight of MST
void Graph::setMSTTotalWeight()
{
    int totalWeight = INIT_INTEGER;
    for (int i = 0; i < this->numVertices; ++i)
    {
        for (int j = i + 1; j < this->numVertices; ++j)
        {
            totalWeight += (*this->mstMatrix)[i][j];
        }
    }
    this->mstTotalWeight = totalWeight;
}

// Return the highest weighted distance in the MST
void Graph::setMSTLongestDistance()
{
    int longestDistance = INIT_INTEGER;
    int numVertices = this->numVertices;

    // Use Floyd-Warshall algorithm to find the longest path in the MST
    std::vector<std::vector<int>> dist = *this->mstMatrix;

    for (int k = 0; k < numVertices; ++k)
    {
        for (int i = 0; i < numVertices; ++i)
        {
            for (int j = 0; j < numVertices; ++j)
            {
                if (dist[i][k] != 0 && dist[k][j] != 0)
                {
                    int newDist = dist[i][k] + dist[k][j]; // new potential distance
                    // check if the current distaance from i to j is either zero or greater than the new potential distance
                    // if either condition is true it updates the distance from i to j to the new larger distance
                    if (dist[i][j] == 0 || newDist < dist[i][j])
                    {
                        dist[i][j] = newDist;
                    }
                }
            }
        }
    }
    // Find the longest distance in the shortest path matrix - after finish updating
    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = i + 1; j < numVertices; ++j)
        {
            if (dist[i][j] > longestDistance)
            {
                longestDistance = dist[i][j];
            }
        }
    }
    this->mstLongestDistance = longestDistance;
}

// Return the average edge weight in the MST
void Graph::setMSTAvgEdgeWeight()
{
    int numVertices = this->getSizeVertices();
    double totalWeight = INIT_DOUBLE;
    double edgeCount = INIT_DOUBLE;

    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = i + 1; j < numVertices; ++j)
        {
            if ((*this->mstMatrix)[i][j] != 0)
            {
                totalWeight += (*this->mstMatrix)[i][j];
                ++edgeCount;
            }
        }
    }

    // Prevent devision by zero
    if (edgeCount == INIT_DOUBLE)
    {
        this->mstAvgEdgeWeight = 0.0;
    }
    this->mstAvgEdgeWeight = (totalWeight / edgeCount);
}

void Graph::setMSTShortestDistance()
{
    int shortestDistance = INT_MAX;
    int numVertices = this->numVertices;

    std::vector<std::vector<int>> dist = *this->mstMatrix; // Use Floyd-Warshall algorithm to find the shortest path in the MST

    for (int k = 0; k < numVertices; ++k)
    {
        for (int i = 0; i < numVertices; ++i)
        {
            for (int j = 0; j < numVertices; ++j)
            {
                if (dist[i][k] != 0 && dist[k][j] != 0)
                {
                    int newDist = dist[i][k] + dist[k][j]; // new potential distance
                    // Check if the current distance from i to j is either zero or greater than the new potential distance
                    // if either condition is true it updates the distance from i to j to the new smaller distance
                    if (dist[i][j] == 0 || newDist < dist[i][j])
                    {
                        dist[i][j] = newDist;
                    }
                }
            }
        }
    }

    // Find the shortest distance in the shortest path matrix - after finish updating
    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = i + 1; j < numVertices; ++j)
        {
            if (dist[i][j] != 0 && dist[i][j] < shortestDistance)
            {
                shortestDistance = dist[i][j];
            }
        }
    }
    this->mstShortestDistance = (shortestDistance == INT_MAX) ? 0 : shortestDistance;
}

// Get String to print of adjacency matrix represent the MST
std::string Graph::printMST() const
{
    std::stringstream mstString;
    for (int i = 0; i < this->numVertices; ++i)
    {
        for (int j = i + 1; j < this->numVertices; ++j)
        {
            if ((*this->mstMatrix)[i][j] != 0)
            {
                mstString << "Edge: " << i << " - " << j << " | Weight: " << (*mstMatrix)[i][j] << "\n";
            }
        }
    }
    return mstString.str();
}