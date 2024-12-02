#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <utility>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include "Pipeline.hpp"
#include "LeaderFollower.hpp"

class Server
{
private:
    std::vector<std::shared_ptr<Graph>> graphsData;                            // Vector to store graphs
    std::vector<std::pair<int, std::unique_ptr<std::thread>>> clients_dataset; // Vector to store client data: socket and thread
    std::mutex clients_mutex;                                                  // Mutex for the clients for
    std::atomic<bool> stopServer{false};                                       // Flag to stop the server
    struct sockaddr_in address{};                                              // Address structure
    int server_fd{-1};                                                         // File descriptor for the server
    Pipeline *pipeline;                                                        // Pointer to the Pipeline pattern
    LeaderFollower *leaderfollower;                                            // Pointer to the Leader-Follower pattern

    void startServer();                    // Start the server
    void handleConnections();              // Handle client connections
    void handleRequest(int client_socket); // Handle client requests
    void sendMessage(int client_FD, const std::string message);
    void graphCreation(int client_FD); // All the progress to create graph and store it (include mst calculation)
    void sendDataToLeaderFollower(int client_FD);
    void sendDataToPipeline(int client_FD); // Send data to Pipeline
    int startConversation(int client_FD);   // Start the conversation with the client

public:
    Server();  // Constructor
    ~Server(); // Destructor
};

#endif