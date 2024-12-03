#include "Server.hpp"

#define PORT 4040
#define INVALID -1
#define NO_MST_DATA_CALCULATION -1

// Constructor
Server::Server(): server_fd(INVALID), pipeline(nullptr), leaderfollower(nullptr), stopServer(false)
{
    std::cout << "Start Building the Server..." << std::endl;
    std::cout << "Starting Pipeline Design Pattern" << std::endl;
    this->pipeline = new Pipeline();
    std::cout << "Starting Leader Follower Design Pattern" << std::endl;
    this->leaderfollower = new LeaderFollower();
    startServer(); // Start the server
}

// Destructor
Server::~Server()
{
    std::cout << "\n********* START Server Stop Process *********" << std::endl;
    delete pipeline;       // Delete the pipeline object
    delete leaderfollower; // Delete the leaderfollower object

    for (auto &client : clients_dataset)
    {
        if (client.first >= 0)
        {
            close(client.first);
        }
        if (client.second->joinable())
        {
            client.second->join();
        }
    }
    clients_dataset.clear();
    std::cout << "Server: All clients File Descriptor are CLOSED" << std::endl;
    std::cout << "Server: All clients Threads are JOINED" << std::endl;
    if (server_fd >= 0)
    {
        close(server_fd);
    }
    std::cout << "Server: Server File Descriptor CLOSE" << std::endl;
    std::cout << "\n********* FINISH Server Stop Process *********" << std::endl;
}

// Start the server
void Server::startServer()
{
    // Creating socket FD
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set the socket options
    int opt = 1; // Enable the socket option
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set the address and port number
    address.sin_family = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Any IP address
    address.sin_port = htons(PORT);       // Port number

    // Bind the socket to the address and port number
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    { // Bind the socket to the address and port number
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started listening on port " << PORT << std::endl;

    // Handle incoming connections
    this->handleConnections();
}

// handle client connections
void Server::handleConnections()
{
    int addrlen = sizeof(this->address);
    fd_set readfds;
    struct timeval timeout;
    int stdin_fd = fileno(stdin);
    int max_fd = std::max(server_fd, stdin_fd);

    while (!stopServer)
    {                      // Loop until the server is stopped
        FD_ZERO(&readfds); // Clear the file descriptor set
        FD_SET(server_fd, &readfds);
        FD_SET(stdin_fd, &readfds);

        // Set timeout to 1 second
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Check for activity on the sockets
        int activity = select(max_fd + 1, &readfds, NULL, NULL, &timeout); // Check for activity on the sockets

        if (activity < 0 && errno != EINTR)
        { // Check for errors
            perror("select error");
            continue;
        }

        // Check for keyboard input
        if (FD_ISSET(stdin_fd, &readfds))
        { // Check if the file descriptor is set
            std::string command;
            std::getline(std::cin, command); // Get the command from the user
            if (command == "stop")
            {
                std::cout << "Command: " << command << std::endl;
                stopServer = true;
                break;
            }
        }

        // Check for new connections
        if (FD_ISSET(server_fd, &readfds))
        {                                                                                           // Check if the file descriptor is set
            int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen); // Accept the connection
            if (new_socket < 0)
            {
                perror("accept failed");
                continue;
            }

            std::lock_guard<std::mutex> lock(this->mtx); // Lock the mutex because we are modifying the clients vector
            std::cout << "New client connected!" << std::endl;
            auto client_thread = std::make_unique<std::thread>(&Server::handleRequest, this, new_socket);
            clients_dataset.emplace_back(new_socket, std::move(client_thread)); // Add client to the list (unique ptr owner is being moved)
            std::cout << "Client has Added to the Clients list!" << std::endl;
        }
    }
}

void Server::handleRequest(int client_FD)
{
    if (client_FD < 0)
    {
        std::perror("Error: Invalid client file descriptor.");
        return;
    }

    char buffer[1024];
    std::string menu =
        "\nMenu:\n"
        "1. Create a New Graph\n"
        "2. Send Data to Pipeline and Active Objects\n"
        "3. Send Data to Leader-Follower\n"
        "4. Print MST Graphs Data\n"
        "0. Exit\n"
        "\nChoice: ";

    while (true)
    {
        try
        {
            sendMessage(client_FD, menu);            // Send the menu to the client
            memset(buffer, 0, sizeof(buffer));       // Clear the buffer
            read(client_FD, buffer, sizeof(buffer)); // Read the client's choice
        
            int choice = 0;
            choice = std::stoi(buffer);
            if (choice < 0 || choice > 4)
            {
                continue;
            }

            switch (choice)
            {
                case 0:
                    stopClient(client_FD);
                    return;

                case 1:
                    graphCreation(client_FD);
                    break;

                case 2:
                    sendDataToPipeline(client_FD);
                    break;

                case 3:
                    sendDataToLeaderFollower(client_FD);
                    break;

                case 4:
                    sendMSTDataToClient(client_FD);
                    break;

                default:
                    sendMessage(client_FD, "Invalid choice. Please try again.\n");
                    break;
            }
        }
        catch (const std::exception &e)
        {
            continue;
        }
    }
}


// Send a message to the client
void Server::sendMessage(int client_FD, const std::string message)
{
    if (client_FD < 0)
    {
        std::perror("Error: Invalid client file descriptor.");
        return;
    }
    send(client_FD, message.c_str(), message.size(), 0);
}

void Server::graphCreation(int client_FD)
{
    sendMessage(client_FD, "Enter the number of vertices: ");
    int numVertices = getIntegerInputFromClient(client_FD);

    if (numVertices <= 0)
    {
        sendMessage(client_FD, "Invalid number of vertices.\n");
        return;
    }

    auto graph = std::make_shared<Graph>(numVertices);
    int numEdges = -1;
    while(numEdges < 0)
    {
        try
        {
            sendMessage(client_FD, "Enter the number of edges: ");
            numEdges = getIntegerInputFromClient(client_FD);
        }
        catch (const std::exception &e)
        {
            sendMessage(client_FD, "Invalid number of edges.\n");
            continue;
        }
    }

    for (int i = 0; i < numEdges; ++i)
    {
        int src = -1, dest = -1, weight = -1;
        while((src < 0 || src >= numVertices) || (dest < 0 || dest >= numVertices))
        {
            try
            {
                sendMessage(client_FD, "Edge - Enter Source / From: ");
                src = getIntegerInputFromClient(client_FD);
                sendMessage(client_FD, "Edge - Enter Destination / To: ");
                dest = getIntegerInputFromClient(client_FD);
                sendMessage(client_FD, "Edge - Enter Weight: ");
                weight = getIntegerInputFromClient(client_FD);
            }
            catch (const std::exception &e)
            {
                sendMessage(client_FD, "Invalid vertices in edge.\n");
                continue;  // retry this edge
            }

            graph->addEdge(src, dest, weight);  // Add the edge to the graph
        }

        src = -1, dest = -1, weight = -1;  // Reset the values
    }

    std::unique_ptr<MSTStrategy> algorithmType;
    while (true)
    {
        sendMessage(client_FD, "Choose MST algorithm:\n"                                "1. Prim's Algorithm\n"
                           "2. Kruskal's Algorithm\nChoice: ");
        int algorithmChoice = getIntegerInputFromClient(client_FD);
        
        if (algorithmChoice == 1)
        {
            algorithmType = std::move(MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Prim));
            break;
        }
        else if (algorithmChoice == 2)
        {
            algorithmType = std::move(MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Kruskal));
            break;
        }
        else
        {
            sendMessage(client_FD, "Invalid algorithm choice.\n");
            continue;
        }
    }

    graph->setMSTStrategy(std::move(algorithmType));  // Set the chosen algorithm
    graph->activateMSTStrategy();  // Store the graph along with the chosen algorithm

    if (graph->getValidationMSTExist())
    {
        std::lock_guard<std::mutex> lock(this->mtx);
        this->vec_SharedPtrGraphs.push_back(graph);
        this->vec_WeakPtrGraphs_Unprocessed.push_back(graph);
        sendMessage(client_FD, "Graph created and stored.\n");
    }
    else
    {
        graph.reset();
        sendMessage(client_FD, "MST does not exist for the given graph.\n");
    }
}

void Server::sendDataToPipeline(int client_FD)
{
    std::lock_guard<std::mutex> lock(this->mtx);
    if(this->vec_WeakPtrGraphs_Unprocessed.size() > 0) filterUnprocessedGraphs();
    this->pipeline->processGraphs(this->vec_WeakPtrGraphs_Unprocessed);
    sendMessage(client_FD, "All graphs have been sent to Pipeline for processing using Active Object.\n");
}

void Server::sendDataToLeaderFollower(int client_FD)
{
    std::lock_guard<std::mutex> lock(this->mtx);
    if(this->vec_WeakPtrGraphs_Unprocessed.size() > 0) filterUnprocessedGraphs();
    this->leaderfollower->processGraphs(this->vec_WeakPtrGraphs_Unprocessed);
    sendMessage(client_FD, "All graphs have been sent to Leader-Follower for processing.\n");
}

void Server::filterUnprocessedGraphs()
{
    std::vector<std::weak_ptr<Graph>> tempWeakGraphs;
    {
        // locking to get shared and access the graph methods and check the status to filter the one that are not yet processed
        auto weakGraphs = this->vec_WeakPtrGraphs_Unprocessed;
        for (const auto &unlockedPtrGraph : weakGraphs)
        {
            if (auto sharedGraph = unlockedPtrGraph.lock()) // Lock the weak pointer to check validity
            {
                if (sharedGraph->getMSTDataStatusCalculation() == NO_MST_DATA_CALCULATION)
                {
                    tempWeakGraphs.push_back(sharedGraph);  // Add the graph to the temporary vector
                }
            }
        }
    }
    if(this->vec_WeakPtrGraphs_Unprocessed.size() != tempWeakGraphs.size()) {
        this->vec_WeakPtrGraphs_Unprocessed = std::move(tempWeakGraphs);  // prevent copying
        std::cout << "Unprocessed Graphs are filtered, remain " << tempWeakGraphs.size() <<" graphs to process"<< std::endl;
    }
}

// Get MST data based on choice
void Server::sendMSTDataToClient(int client_FD)
{
    if (client_FD < 0)
    {
        std::perror("Error: Invalid client file descriptor.");
        return;
    }
    int counter = 0; // Number of graphs start from 1 (increase in every loop - also the first one)
    for (auto myGraph : this->vec_SharedPtrGraphs)
    {
        counter++; // Increase Number of graphs (Starting from 1)
        std::string message = "********* Graph Number " + std::to_string(counter) + " *********.\n ";
        sendMessage(client_FD, message);
        if (myGraph == nullptr)
        {
            continue;
        }
        else if (!myGraph->getValidationMSTExist())
        {
            message = "MST is not computed. Please pass it to Pipeline or Leader-Follower.\n";
            sendMessage(client_FD, message);
            continue;
        }
        else if(myGraph->getMSTDataStatusCalculation() == NO_MST_DATA_CALCULATION)
        {
            message = "MST is not computed. Please pass it to Pipeline or Leader-Follower.\n";
            sendMessage(client_FD, message);
            continue;
        }
        message = "Weight of the longest path in MST: " + std::to_string(myGraph->getMSTLongestDistance()) + "\n";
        message += "Weight of the shortest path in MST: " + std::to_string(myGraph->getMSTShortestDistance()) + "\n";
        message += "Average weight of the edges in MST: " + std::to_string(myGraph->getMSTAvgEdgeWeight()) + "\n";
        message += "Total weight of the MST: " + std::to_string(myGraph->getMSTTotalWeight()) + "\n";
        message += "MST Edge Printing (Not Part Of Design Patterns Process):\n" + myGraph->printMST();
        sendMessage(client_FD, message);
    }
}

void Server::stopClient(int client_FD)
{ // Stop the client connection
    if (client_FD < 0)
    {
        std::perror("Error: Invalid client file descriptor.");
    }
    close(client_FD);
    std::cout << "Client Connection Closed" << std::endl;
}

int Server::getIntegerInputFromClient(int client_FD)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(client_FD, buffer, sizeof(buffer));
    int data = INVALID;
    try
    {
        data = std::stoi(buffer);
    }
    catch (std::invalid_argument &e)
    {
        return INVALID;
    }
    return data;
}

std::string Server::getStringInputFromClient(int client_FD)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(client_FD, buffer, sizeof(buffer));
    return std::string(buffer);
}

int main(int argc, char *argv[])
{
    Server *serverObj = new Server();
    delete serverObj;
    return 0;
}