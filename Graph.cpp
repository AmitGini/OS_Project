#include "Graph.hpp"


// Constructor
Graph::Graph(int vertices) : numVertices(vertices), numEdges(0),
adjMatrix(vertices, std::vector<int>(vertices, 0)), mstMatrix(nullptr) {}

// Destructor
Graph::~Graph(){
    delete mstMatrix; // Clean up the allocated memory
}
 
// Add edge to graph
void Graph::addEdge(int u, int v, int weight){
    if(u < 0 || u >= this->numVertices || v < 0 || v >= this->numVertices){
        //throw std::runtime_error("Invalid vertices");
        return;
    }
    else if(this->adjMatrix[u][v] != 0 || this->adjMatrix[v][u] != 0){
        this->adjMatrix[u][v] = weight;
        this->adjMatrix[v][u] = weight; // Assuming undirected graph
        return;
    }else{
        this->adjMatrix[u][v] = weight;
        this->adjMatrix[v][u] = weight; // Assuming undirected graph
        this->numEdges++;
    }
}

// Remove edge from graph
void Graph::removeEdge(int u, int v){
    if(u < 0 || u >= this->numVertices || v < 0 || v >= this->numVertices){
        //throw std::runtime_error("Invalid vertices");
        return;
    }else if(this->adjMatrix[u][v] == 0 || this->adjMatrix[v][u] == 0){
        return;
    }else{
        this->adjMatrix[u][v] = 0;
        this->adjMatrix[v][u] = 0;
        this->numEdges--;
    }
}

// Get weight of edge
int Graph::getEdgeWeight(int u, int v) const{
    return this->adjMatrix[u][v];
}

// Get number of vertices
int Graph::getSizeVertices() const{
    return this->numVertices;
}

int Graph::getNumEdges() const
{
    return this->numEdges;
}

// Get adjacency matrix represent the graph
const std::vector<std::vector<int>> &Graph::getGraph() const{
    return this->adjMatrix;
}

// Set adjacency matrix represent the MST
void Graph::setMST(std::vector<std::vector<int>>* mst){
    if(mst == nullptr){
        //throw std::runtime_error("Invalid matrix size");
        return;
    }else if(mstMatrix != nullptr){
        delete this->mstMatrix;
        this->mstMatrix = nullptr;
    }

    this->mstMatrix = mst;
}

// Calculate and return the total weight of MST
int Graph::getMSTTotalWeight() const
{
    if(this->mstMatrix == nullptr){
        std::cerr << "MST matrix is not set\n";
        return 0;
    }
    
    int totalWeight = 0;
    for(int i = 0; i < this->numVertices; ++i){
        for(int j = i + 1; j < this->numVertices; ++j){
            totalWeight += (*this->mstMatrix)[i][j];
        }
    }
    return totalWeight;
}

// Return the highest weighted distance in the MST
int Graph::getMSTLongestDistance() const {
    if (this->mstMatrix == nullptr) {
        std::cerr << "MST matrix is not set\n";
        return 0;
    }

    int longestDistance = 0;
    int numVertices = this->numVertices;

    // Use Floyd-Warshall algorithm to find the longest path in the MST
    std::vector<std::vector<int>> dist = *this->mstMatrix;

    for (int k = 0; k < numVertices; ++k) {
        for (int i = 0; i < numVertices; ++i) {
            for (int j = 0; j < numVertices; ++j) {
                if (dist[i][k] != 0 && dist[k][j] != 0) {
                    int newDist = dist[i][k] + dist[k][j]; // new potential distance
                    //check if the current distaance from i to j is either zero or greater than the new potential distance
                    //if either condition is true it updates the distance from i to j to the new larger distance
                    if (dist[i][j] == 0 || newDist < dist[i][j]) {
                        dist[i][j] = newDist;
                    }
                }
            }
        }
    }

    // Find the longest distance in the shortest path matrix - after finish updating
    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            if (dist[i][j] > longestDistance) {
                longestDistance = dist[i][j];
            }
        }
    }

    return longestDistance;
}

//Return the average edge weight in the MST
double Graph::getMSTAvgEdgeWeight() const{
    if(this->mstMatrix == nullptr){
        std::cerr << "MST matrix is not set\n";
        return 0.0;
    }

    int numVertices = this->getSizeVertices();
    double totalWeight = 0.0;
    int edgeCount = 0;

    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            if ((*this->mstMatrix)[i][j] != 0) {
                totalWeight += (*this->mstMatrix)[i][j];
                ++edgeCount;
            }
        }
    }
    if (edgeCount == 0) {
        return 0.0;
    }
    
    return totalWeight / edgeCount;
}

int Graph::getMSTShortestDistance() const {
    if (this->mstMatrix == nullptr) {
        std::cerr << "MST matrix is not set\n";
        return 0;
    }

    int shortestDistance = INT_MAX;
    int numVertices = this->numVertices;

    // Use Floyd-Warshall algorithm to find the shortest path in the MST
    std::vector<std::vector<int>> dist = *this->mstMatrix;

    for (int k = 0; k < numVertices; ++k) {
        for (int i = 0; i < numVertices; ++i) {
            for (int j = 0; j < numVertices; ++j) {
                if (dist[i][k] != 0 && dist[k][j] != 0) {
                    int newDist = dist[i][k] + dist[k][j]; // new potential distance
                    // Check if the current distance from i to j is either zero or greater than the new potential distance
                    // if either condition is true it updates the distance from i to j to the new smaller distance
                    if (dist[i][j] == 0 || newDist < dist[i][j]) {
                        dist[i][j] = newDist;
                    }
                }
            }
        }
    }

    // Find the shortest distance in the shortest path matrix - after finish updating
    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            if (dist[i][j] != 0 && dist[i][j] < shortestDistance) {
                shortestDistance = dist[i][j];
            }
        }
    }

    return (shortestDistance == INT_MAX) ? 0 : shortestDistance;
}

std::string Graph::printMST() const {
    if(this->mstMatrix == nullptr){
        return "MST matrix is not set\n";
    }

    std::stringstream mstString;
    for (int i = 0; i < this->numVertices; ++i) {
        for (int j = i + 1; j < this->numVertices; ++j) {
            if ((*this->mstMatrix)[i][j] != 0) {
                mstString << "Edge: " << i << " - " << j << " | Weight: " << (*mstMatrix)[i][j] << "\n";
            }
        }
    }
    return mstString.str();
}