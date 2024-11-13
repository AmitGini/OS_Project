#include "RequestService.hpp"


// Send a message to the client
void RequestService::sendMessage(int &client_FD, const std::string& message) {
    if(client_FD < 0) {
        std::perror("Error: Invalid client file descriptor.");
        return;
    }
    send(client_FD, message.c_str(), message.size(), 0);
}

// Start the conversation with the client
int RequestService::startConversation(int &client_FD){
    if(client_FD < 0) {
        std::perror("Error: Invalid client file descriptor.");
        return -1;
    }
    std::string menu =
        "\nMenu:\n"
        "0. Execute Tasks      (LF ONLY!)\n"
        "1. Create a New Graph\n"
        "2. Add Edge\n"
        "3. Remove Edge\n"
        "4. Compute MST\n"
        "5. Get MST Weighted Longest Path \n"
        "6. Get MST Weighted Shortest Path\n"
        "7. Get MST Average Weight of Edge\n"
        "8. Get MST Total Weight of Edges \n"
        "9. Print MST\n"
        "10. Exit\n"
        "\nChoice: ";
    
    while (true) {
        sendMessage(client_FD, menu); // Send the menu to the client
        memset(buffer, 0, sizeof(buffer));  // Clear the buffer
        read(client_FD, buffer, sizeof(buffer));  // Read the client's choice
        int choice = 0;
        try{
            choice = std::stoi(buffer);
            if(choice < 0 || choice > 10){
                throw std::invalid_argument("Invalid choice");
            }
        }catch(std::invalid_argument& e){
            sendMessage(client_FD, "Invalid choice. Please try again.\n");
            continue;
        }
        memset(buffer, 0, sizeof(buffer));
        return choice;
    }
}

// Create a new graph conversation
bool RequestService::createGraph(int &client_FD){
    if(client_FD < 0) {
        std::perror("Error: Invalid client file descriptor.");
        return false;
    }
    std::lock_guard<std::mutex> lock(graphMutex);
    std::string message = "Enter the number of vertices: ";
    sendMessage(client_FD, message);

    int numVertices = 0;
    memset(buffer, 0, sizeof(buffer));
    read(client_FD, buffer, sizeof(buffer)); // Read number of vertices from client
    try{
        numVertices = std::stoi(buffer);
        memset(buffer, 0, sizeof(buffer));
        if(numVertices <= 0){  
            throw std::invalid_argument("Invalid number of vertices");
        }

        graph = std::make_unique<Graph>(numVertices);  // Create a new graph
        
        // Send acknowledgment to client
        message = "Graph created with " + std::to_string(numVertices) + " vertices.\n";
        sendMessage(client_FD, message);
        return true;
    
    }catch(std::invalid_argument& e){
        message = "Invalid number of vertices, Exiting Graph Creation.\n";
        sendMessage(client_FD, message);
        memset(buffer, 0, sizeof(buffer));
        return false;
    }
}

// Add edge to the graph conversation
bool RequestService::modifyGraph(int &client_FD, bool toAddEdge) {
    if(client_FD < 0) {
        std::perror("Error: Invalid client file descriptor.");
        return false;
    }

    std::lock_guard<std::mutex> lock(graphMutex);
    if(this->graph == nullptr){
        std::string message = "Graph is not created. Please create a graph first.\n";
        sendMessage(client_FD, message);
        return false;
    }

    int graphSize = graph->getSizeVertices();
    memset(buffer, 0, sizeof(buffer));
    std::string message;

    if(toAddEdge){        
        int u = 0, v = 0, weight = 0;
        message = "Enter the source vertex, destination vertex, and edge weight separated by spaces: ";
        sendMessage(client_FD, message);
        read(client_FD, buffer, sizeof(buffer));  // Read edge information from client
        
        if (sscanf(buffer, "%d %d %d", &u, &v, &weight) != 3 || u < 0 || v < 0 || weight < 0 || u >= graphSize || v >= graphSize) {
            message = "Invalid input. Please enter three integers separated by spaces.\n";
            sendMessage(client_FD, message);
            memset(buffer, 0, sizeof(buffer));
            return false;
        }
        
        graph->addEdge(u, v, weight); // Add edge from graph
        message = "Edge added between " + std::to_string(u) + " and " + std::to_string(v) + " with weight " + std::to_string(weight) + ".\n";
        sendMessage(client_FD, message); // Send acknowledgment to client
    }else{
        message = "Enter the source vertex, destination vertex, separated by spaces: ";
        sendMessage(client_FD, message);
        read(client_FD, buffer, sizeof(buffer)); // Read edge information from client
        
        int u = 0, v = 0;
        if (sscanf(buffer, "%d %d", &u, &v) != 2 || u < 0 || v < 0 || u >= graphSize || v >= graphSize) {
            message = "Invalid input. Please enter two integers separated by spaces.\n";
            sendMessage(client_FD, message);
            memset(buffer, 0, sizeof(buffer));
            return false;
        }

        graph->removeEdge(u, v); // Remove edge from graph
        message = "Edge removed between " + std::to_string(u) + " and " + std::to_string(v) + ".\n";
        sendMessage(client_FD, message);  // Send acknowledgment to client
    }
    memset(buffer, 0, sizeof(buffer));
    return true;
}

// Calculate MST conversation
bool RequestService::calculateMST(int &client_FD) {
    if(client_FD < 0) {
        std::perror("Error: Invalid client file descriptor.");
        return false;
    }

    std::lock_guard<std::mutex> lock(graphMutex);
    if (this->graph == nullptr) {
        std::string message = "Graph is not created. Please create a graph first.\n";
        sendMessage(client_FD, message);
        return false;
    }
    
    std::string message = "Choose MST algorithm:\n1. Prim's Algorithm\n2. Kruskal's Algorithm\n";
    sendMessage(client_FD, message);
    memset(buffer, 0, sizeof(buffer));
    read(client_FD, buffer, sizeof(buffer)); // Read the client's choice
    
    int choice = 0;
    try {
        choice = std::stoi(buffer);
        memset(buffer, 0, sizeof(buffer));

        if (choice < 1 || choice > 2) {
            throw std::invalid_argument("Invalid choice");
        }
    } catch (std::invalid_argument& e) {
        sendMessage(client_FD, "Invalid choice. Please try again.\n");
        memset(buffer, 0, sizeof(buffer));
        return false;
    }
    
    std::unique_ptr<MSTStrategy> strategy = nullptr;
    if (choice == 1) {
        strategy = MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Prim);
    } else if (choice == 2) {
        strategy = MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Kruskal);
    }
    
    if (strategy) {
        std::unique_ptr<std::vector<std::vector<int>>> mst = strategy->computeMST(*graph);
        if(mst){
            graph->setMST(std::move(mst));
            message = "MST computed using " + std::string(choice == 1 ? "Prim's" : "Kruskal's") + " Algorithm.\n";
            sendMessage(client_FD, message);
        } else {
            return false;
        }
    }
    return true;
}

// Get MST data based on choice
bool RequestService::getMSTData(int &client_FD, int choice) {
    if(client_FD < 0) {
        std::perror("Error: Invalid client file descriptor.");
        return false;
    }

    std::lock_guard<std::mutex> lock(graphMutex);
    if(this->graph == nullptr){
        std::string message = "Graph is not created. Please create a graph first.\n";
        sendMessage(client_FD, message);
        return false;
    }else if(!this->graph->hasMST()){
        std::string message = "MST is not computed. Please compute MST first.\n";
        sendMessage(client_FD, message);
        return false;
    }

    std::string message;
    double data = -1;
    std::string mst_data = "";
    switch (choice) {
        case 5:   // Get the weight of the longest path in MST
            data = static_cast<int>(graph->getMSTLongestDistance());
            message = "Weight of the longest path in MST: " + std::to_string(data) + "\n";
            break;
        
        case 6:  // Get the weight of the shortest path in MST
            data = static_cast<int>(graph->getMSTShortestDistance());
            message = "Weight of the shortest path in MST: " + std::to_string(data) + "\n";
            break;
        
        case 7:  // Get the average weight of the edges in MST
            data = graph->getMSTAvgEdgeWeight();
            message = "Average weight of the edges in MST: " + std::to_string(data) + "\n";
            break;
        
        case 8:  // Total Weight of the MST
            data = static_cast<int>(graph->getMSTTotalWeight());
            message = "Total weight of the MST: " + std::to_string(data) + "\n";
            break;
        
        case 9: // Print MST
            mst_data = graph->printMST();
            message = "MST:\n" + mst_data + "\n";
            break;
        
        default: // Invalid choice
            return false;
    }
    sendMessage(client_FD, message);
    return true;
}

bool RequestService::stopClient(int &client_FD) {
    if (client_FD < 0) {
        std::perror("Error: Invalid client file descriptor.");
        return false;
    }
    std::cout << "Client Connection Closed" << std::endl;
    return true;
}