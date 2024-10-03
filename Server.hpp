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
#include "RequestService.hpp"
#include "PipelineDP.hpp"
#include "LeaderFollowerDP.hpp"

#define PORT 4040

class Server {
    
    private:
        RequestService* patternType;
        //LeaderFollowerDP* leaderFollower;
        struct sockaddr_in address;
        int server_fd, new_socket, addrlen;;
        std::vector<int> client_sockets;
    
    public:
        Server(bool isPipelineDP);  // Constructor
        ~Server();  // Destructor
        void start();  // Start the server
        void handleConnections();
};


#endif