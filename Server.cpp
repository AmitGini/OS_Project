#include "Server.hpp"

// Constructor 
Server::Server(bool isPipe): patternType(nullptr) , server_fd(-1), new_socket(-1), stopServer(false){
    std::cout<<"Start Building the Server..."<<std::endl;
    this->stopThread = std::thread(&Server::stop, this); // Start the stop thread - to listen for stop command input from keyboard
    stopThread.detach();
    
    // Choose the Design pattern
    if(isPipe){
        std::cout<<"Starting Pipeline Design Pattern"<<std::endl;
        this->patternType = new PipeDP();
    }else{
        std::cout<<"Starting Leader Follower Design Pattern"<<std::endl;
        this->patternType = new LeaderFollowerDP();
    }

    start();  // Start the server
}

// Destructor
Server::~Server(){    
    std::cout<<"\n*********Closing Progress has Started*********"<<std::endl;
    std::cout<<"Closing Pattern Type..."<<std::endl;
    delete patternType;

    std::cout<<"Closing Server..."<<std::endl;

    std::cout<<"Closing Client Sockets && Join Threads..."<<std::endl;
    for(std::pair<int,std::thread>& client_data : clients_dataset){
        if (client_data.first >= 0) {
            close(client_data.first);
        }
        
        if(client_data.second.joinable()){
            client_data.second.join();
        }
    }

    clients_dataset.clear();

    std::cout<<"Closing Server File Descriptor..."<<std::endl;
    if (server_fd >= 0) {
        close(server_fd);
    }

    std::cout<<"Closing Server Stop Thread..."<<std::endl;
    if(stopThread.joinable()){
        stopThread.join();
    }
    
    std::cout<<"Server has Closed!"<<std::endl;
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

void Server::stop(){
    std::string command;
    while(true){
        std::cin >> command;
        if (command == "stop") {
            this->stopServer = true;
            break;
        }
    }

    delete this;
}

void Server::handleConnections()
{
    int addrlen = sizeof(this->address);
    while (true) {
        this->new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            if(stopServer) break;
            continue;
        }
        std::cout << "New client connected!" << std::endl;
        std::thread client_thread(&Server::acceptClientAccess, this, new_socket);
        client_thread.detach();  // Handle the client in a separate thread
        clients_dataset.push_back(std::make_pair(new_socket, std::move(client_thread)));
        std::cout << "Client has Added to the Clients list!" << std::endl;
        
        if(stopServer) break;
    }
}

void Server::acceptClientAccess(int client_socket){
    if(client_socket < 0){
        return;
    }

    std::cout << "Accepted New Connection to the Program - New Client" << std::endl;
    this->patternType->handleRequest(client_socket);  // Each request handled by the pipeline
    
    
    auto it = std::find_if(clients_dataset.begin(), clients_dataset.end(), 
                            [client_socket](const std::pair<int, std::thread>& element) { return element.first < 0; });
    if (it != clients_dataset.end()) {
        std::cout<<"\n*********Client Exited the Program*********"<<std::endl;
        if (it->second.joinable()) {
            it->second.join();
            std::cout<<"Exited Client Thread has Joined"<<std::endl;
        }
        // Remove the client from the dataset
        clients_dataset.erase(it);
        std::cout<<"Exited Client has Removed from the Clients list"<<std::endl;
        std::cout<<"*********************************************\n"<<std::endl;
    }
}