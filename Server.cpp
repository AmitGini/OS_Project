#include "Server.hpp"

// Constructor 
// ** Need to add leaderfollower **
Server::Server(bool isPipelineDP): patternType(nullptr) ,addrlen(sizeof(this->address)), server_fd(-1), new_socket(-1){
    std::cout<<"Start Building the Server..."<<std::endl;
    if(isPipelineDP){
        std::cout<<"Starting Pipeline Design Pattern"<<std::endl;
        this->patternType = new PipelineDP();
    }else{
        std::cout<<"Starting Leader Follower Design Pattern"<<std::endl;
        this->patternType = new LeaderFollowerDP();
    }
}

Server::~Server(){
    delete patternType;
    
    for(int fd :client_sockets){
        if (fd >= 0) {
            close(fd);
        }
    }
    client_sockets.clear();

    if (server_fd >= 0) {
        close(server_fd);
    }
}

// Start the server
void Server::start() {
    // Creating socket FD
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set the address and port number
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the address and port number
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started listening on port " << PORT << std::endl;

    // Handle incoming connections
    this->handleConnections();
}

void Server::handleConnections()
{
    while (true) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        std::cout << "New client connected!" << std::endl;
        client_sockets.push_back(new_socket);  // Add the client socket to the list of client sockets

        // client socket closed when ever he exits in the convesation
        this->patternType->handleRequest(new_socket);  // Each request handled by the pipeline
    }
}
