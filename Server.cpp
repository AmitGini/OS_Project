#include "Server.hpp"

// Constructor 
Server::Server(bool isPipe) {
    std::cout<<"Start Building the Server..."<<std::endl;

    // Choose the Design pattern
    if(isPipe){
        std::cout<<"Starting Pipeline Design Pattern"<<std::endl;
        this->patternType = std::make_unique<PipeDP>();
    }else{
        std::cout<<"Starting Leader Follower Design Pattern"<<std::endl;
        this->patternType = std::make_unique<LeaderFollowerDP>();
    }



    start();  // Start the server
}

// Destructor
Server::~Server(){    
    std::cout << "\n********* Stopping Server *********" << std::endl;

    clients_dataset.clear();
    patternType.reset();

    std::cout<<"Closing Server File Descriptor..."<<std::endl;
    if (server_fd >= 0) {
        close(server_fd);
    }

    std::cout<<"********* Server has Closed! *********"<<std::endl;
}

// Start the server
void Server::start() {
    // Creating socket FD
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set the socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
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

void Server::handleConnections() {
    int addrlen = sizeof(this->address);
    fd_set readfds;
    struct timeval timeout;
    int stdin_fd = fileno(stdin);
    int max_fd = std::max(server_fd, stdin_fd);
    
    while(!stopServer) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        FD_SET(stdin_fd, &readfds);

        // Set timeout to 1 second
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, &timeout);

        if(activity < 0 && errno != EINTR) {
            perror("select error");
            continue;
        }

        // Check for keyboard input
        if(FD_ISSET(stdin_fd, &readfds)) {
            std::string command;
            std::getline(std::cin, command);
            if(command == "stop") {
                std::cout << "Command: " << command << std::endl;
                stopServer = true;
                break;
            }
        }

        // Check for new connections
        if(FD_ISSET(server_fd, &readfds)) {
            int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (new_socket < 0) {
                perror("accept failed");
                continue;
            }
            
            std::lock_guard<std::mutex> lock(clients_mutex);
            std::cout << "New client connected!" << std::endl;
            auto client_thread = std::make_shared<std::thread>(
                &Server::acceptClientAccess, this, new_socket);
            clients_dataset.emplace_back(new_socket, std::move(client_thread));
            std::cout << "Client has Added to the Clients list!" << std::endl;
        }
    }
}
void Server::acceptClientAccess(int client_socket){
    if(client_socket < 0) return;
    try {
        this->patternType->handleRequest(client_socket);
    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = std::find_if(clients_dataset.begin(), clients_dataset.end(),
            [client_socket](const auto& pair) { return pair.first == client_socket; });
            
        if (it != clients_dataset.end()) {
            if(it->second && it->second->joinable()) {
                it->second->detach();
            }
            // Close socket before removing from vector
            if (close(client_socket) < 0) {
                std::cerr << "Error closing socket: " << strerror(errno) << std::endl;
            }
            clients_dataset.erase(it);
        }
    }

}