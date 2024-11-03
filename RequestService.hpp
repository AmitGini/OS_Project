#ifndef REQUESTSERVICE_HPP
#define REQUESTSERVICE_HPP

#include <sstream>
#include <sys/socket.h>  // Include socket headers - for tasks
#include <unistd.h>
#include <memory>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <mutex>
#include "MSTFactory.hpp"
#include "MSTStrategy.hpp"
#include "PrimStrategy.hpp"
#include "KruskalStrategy.hpp"
#include "Graph.hpp"

// Abstract class providing an interface for request handling services.
class RequestService {
protected:
    std::mutex graphMutex;  // Mutex for graph operations
    Graph* graph = nullptr;  // Pointer to a Graph object
    char buffer[1024];  // Buffer for reading and writing data

public:
    virtual ~RequestService() = default;  // Virtual destructor to ensure proper cleanup in derived classes
    virtual void handleRequest(int& client_FD) = 0;  // Pure virtual function to be implemented by derived classes to handle requests
    void sendMessage(int &client_FD, const std::string& message);  // Sends a message to a client
    int startConversation(int &client_FD);  // Initiates conversation with a client, presenting a menu and handling input
    bool createGraph(int &client_FD);  // Handles the creation of a new graph
    bool modifyGraph(int &client_FD, bool toAddEdge);  // Modifies the graph by adding or removing an edge
    bool calculateMST(int &client_FD);  // Calculates the MST using a chosen algorithm
    bool getMSTData(int &client_FD, int choice);  // Fetches and sends data related to the MST based on client's choice
    bool stopClient(int &client_FD);  // Stops the client connection

};
#endif