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

     // Start the stop thread - to listen for stop command input from keyboard
    auto stop_thread = std::make_shared<std::thread>(&Server::stop, this);  
    stop_thread->detach();  // Detach the thread

    start();  // Start the server
}

// Destructor
Server::~Server(){    
    std::cout << "\n********* Stopping Server *********" << std::endl;
    std::cout<<"Closing Client Sockets && Join Threads..."<<std::endl;
    for(auto& client_data : clients_dataset){
        if (client_data.first > 0) {
            close(client_data.first);
        }
        
        if(client_data.second && client_data.second->joinable()){
            client_data.second->join();
        }
    }

    clients_dataset.clear();

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

void Server::stop(){
    std::string command;
    while(true){
        std::cin >> command;
        if (command == "stop") {
            this->stopServer = true;
            break;
        }
    }
}

void Server::handleConnections(){
    int addrlen = sizeof(this->address);
    fd_set readfds;
    struct timeval timeout;
    
    while(!stopServer) {
        int new_socket = -1;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        // Set timeout to 1 second
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);

        if(activity < 0 && errno != EINTR) {
            perror("select error");
        }

        if(activity == 0) {
            // Timeout occurred, check if we should stop
            if (stopServer) break;
            continue;
        }

        if(FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (new_socket < 0) {
                perror("accept failed");
                if(stopServer) break;
                continue;
            }
        }

        if(new_socket >= 0) {
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