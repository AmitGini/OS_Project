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
#include "RequestService.hpp"
#include "PipeDP.hpp"
#include "LeaderFollowerDP.hpp"
  

#define PORT 4040

class Server {
public:
    Server(bool isPipe);  // Constructor
    ~Server();  // Destructor
    
    void stop();  // Stop the server    

private:
    std::vector<std::pair<int,std::thread>> clients_dataset;  // Vector to store client data: socket and thread
    RequestService* patternType;  // Pointer to the thread design pattern 

    struct sockaddr_in address;  // Address structure
    int server_fd;  // File descriptor for the server
    
    bool stopServer;  // Flag to stop the server
    std::mutex clients_mutex;

    void start();  // Start the server
    void handleConnections();  // Handle client connections
    void acceptClientAccess(int client_socket);  // Initialize the client 
};


#endif