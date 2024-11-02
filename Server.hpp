#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include "RequestService.hpp"
#include "PipeDP.hpp"
#include "LeaderFollowerDP.hpp"

#define PORT 4040

class Server {
    
    private:
        RequestService* patternType;  // Pointer to the thread design pattern 
        struct sockaddr_in address;  // Address structure
        int server_fd, new_socket;  // File descriptors
        std::vector<std::pair<int,std::thread>> clients_dataset;  // Vector to store client data: socket and thread
        std::thread stopThread;  // Thread to stop the server
        bool stopServer;  // Flag to stop the server

        void start();  // Start the server
        void acceptClientAccess(int client_socket);  // Initialize the client 
    
    public:
        Server(bool isPipelineDP);  // Constructor
        ~Server();  // Destructor
        
        void stop();  // Stop the server    
        void handleConnections();  // Handle client connections
};


#endif