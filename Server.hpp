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
#include <memory>
#include "RequestService.hpp"
#include "PipeDP.hpp"
#include "LeaderFollowerDP.hpp"
  

#define PORT 4040

class Server {
private:
    std::vector<std::pair<int, std::unique_ptr<std::thread>>> clients_dataset;  // Vector to store client data: socket and thread
    std::unique_ptr<RequestService> patternType;  // Pointer to the thread design pattern 
    std::unique_ptr<std::thread> stop_thread;  // Pointer to the stop thread
    std::mutex clients_mutex;
    std::atomic<bool> stopServer{false};  // Flag to stop the server
    struct sockaddr_in address{};  // Address structure
    int server_fd{-1};  // File descriptor for the server

    void start();  // Start the server
    void handleConnections();  // Handle client connections
    void acceptClientAccess(int client_socket);  // Initialize the client 
public:
    Server(bool isPipe);  // Constructor
    ~Server();  // Destructor
    
    void stop();  // Stop the server    


};


#endif