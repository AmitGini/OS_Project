#include "Server.hpp"

#define PORT 4040
#define INVALID -1

// Constructor
Server::Server()
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
    std::cout << "\n********* Starting Stop Process *********" << std::endl;
    delete pipeline;       // Delete the pipeline object
    delete leaderfollower; // Delete the leaderfollower object

    clients_dataset.clear();

    std::cout << "Closing Server File Descriptor..." << std::endl;
    if (server_fd >= 0)
    {
        close(server_fd);
    }

    std::cout << "********* Server has Closed! *********" << std::endl;
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

            std::lock_guard<std::mutex> lock(clients_mutex); // Lock the mutex because we are modifying the clients vector
            std::cout << "New client connected!" << std::endl;
            auto client_thread = std::make_shared<std::thread>(
                &Server::handleRequest, this, new_socket);
            clients_dataset.emplace_back(new_socket, std::move(client_thread));
            std::cout << "Client has Added to the Clients list!" << std::endl;
        }
    }
}

void Server::handleRequest(int client_socket)
{
    if (client_socket > 0)
    {
        try
        {
            while (true)
            {
                int choice = startConversation(client_socket);

                switch (choice)
                {
                case 0:
                    stopClient(client_socket);
                    return;

                case 1:
                    graphCreation(client_socket);
                    break;

                case 2:
                    sendDataToPipeline(client_socket);
                    break;

                case 3:
                    sendDataToLeaderFollower(client_socket);
                    break;

                case 4:
                    printMSTGraphsData(client_socket);
                    break;

                default:
                    sendMessage(client_socket, "Invalid choice. Please try again.\n");
                    break;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error handling client: " << e.what() << std::endl;
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

// Start the conversation with the client
int Server::startConversation(int lient_FD)
{
    if (client_FD < 0)
    {
        std::perror("Error: Invalid client file descriptor.");
        return -1;
    }

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
        sendMessage(client_FD, menu);            // Send the menu to the client
        memset(buffer, 0, sizeof(buffer));       // Clear the buffer
        read(client_FD, buffer, sizeof(buffer)); // Read the client's choice
        int choice = 0;
        try
        {
            choice = std::stoi(buffer);
            if (choice < 0 || choice > 4)
            {
                throw std::invalid_argument("Invalid choice");
            }
        }
        catch (std::invalid_argument &e)
        {
            sendMessage(client_FD, "Invalid choice. Please try again.\n");
            continue;
        }
        memset(buffer, 0, sizeof(buffer));
        return choice;
    }
}

void Server::graphCreation(int client_FD)
{
    sendMessage(client_FD, "Enter the number of vertices: ");
    int numVertices = receiveIntFromClient(client_FD);

    if (numVertices <= 0)
    {
        sendMessage(client_FD, "Invalid number of vertices.\n");
        return;
    }

    auto graph = std::make_unique<Graph>(numVertices);

    sendMessage(client_FD, "Enter the number of edges: ");
    int numEdges = receiveIntFromClient(client_FD);

    if (numEdges < 0)
    {
        sendMessage(client_FD, "Invalid number of edges.\n");
        graph.release();
        return;
    }

    for (int i = 0; i < numEdges; ++i)
    {
        sendMessage(client_FD, "Enter edge (format: src dest weight): ");
        std::string edgeInput = receiveStringFromClient(client_FD);

        int src, dest, weight;
        std::istringstream iss(edgeInput);
        if (!(iss >> src >> dest >> weight))
        {
            sendMessage(client_FD, "Invalid edge format.\n");
            --i; // retry this edge
            continue;
        }

        try
        {
            graph->addEdge(src, dest, weight);
        }
        catch (const std::out_of_range &e)
        {
            sendMessage(client_FD, "Invalid vertices in edge.\n");
            --i; // retry this edge
            continue;
        }
    }

    while (true)
    {
        sendMessage(client_FD, "Choose MST algorithm:\n1. Prim's Algorithm\n2. Kruskal's Algorithm\nChoice: ");
        int algorithmChoice = receiveIntFromClient(client_FD);

        MSTFactory::AlgorithmType algorithmType;
        if (algorithmChoice == 1)
        {
            algorithmType = MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Prim);
            break;
        }
        else if (algorithmChoice == 2)
        {
            algorithmType = MSTFactory::createMSTStrategy(MSTFactory::AlgorithmType::Kruskal);
            break;
        }
        else
        {
            sendMessage(client_FD, "Invalid algorithm choice.\n");
            continue;
        }
    }

    // Store the graph along with the chosen algorithm
    graph->setMSTAlgorithmType(strategy->computeMST(graph););

    if (graph->getValidationMSTExist())
    {
        std::lock_guard<std::mutex> lock(graphs_mutex);
        graphsData.push_back(std::move(graph));
        sendMessage(client_FD, "Graph created and stored.\n");
    }
    else
    {
        graph.release();
        sendMessage(client_FD, "MST does not exist for the given graph.\n");
    }
}

void Server::sendDataToPipeline(int client_FD)
{
    // For each graph, send it to the Pipeline pattern for processing
    std::lock_guard<std::mutex> lock(graphs_mutex);

    for (auto &graph : graphsData)
    {
        patternType->processGraph(*graph); // Assuming processGraph is defined in RequestService
    }

    sendMessage(client_FD, "All graphs have been send to Pipeline for process using Active Object.\n");
}

void Server::sendDataToLeaderFollower(int client_FD)
{
    // For each graph, send it to the Leader-Follower pattern for processing
    std::lock_guard<std::mutex> lock(graphs_mutex);

    for (auto &graph : graphsData)
    {
        patternType->processGraph(*graph); // Assuming processGraph is defined in RequestService
    }

    sendMessage(client_FD, "All graphs have been send to Leader-Follower for process.\n");
}

// Get MST data based on choice
bool Server::getMSTData(int client_FD)
{
    if (client_FD < 0)
    {
        std::perror("Error: Invalid client file descriptor.");
        return false;
    }

    std::lock_guard<std::mutex> lock(graphMutex);
    if (this->graph == nullptr)
    {
        std::string message = "Graph is not created. Please create a graph first.\n";
        sendMessage(client_FD, message);
        return false;
    }
    else if (!this->graph->getValidationMSTExist())
    {
        std::string message = "MST is not computed. Please compute MST first.\n";
        sendMessage(client_FD, message);
        return false;
    }

    std::string message = "Weight of the longest path in MST: " + graph->getMSTLongestDistance() + "\n";
    sendMessage(client_FD, message);
    message = "Weight of the shortest path in MST: " + graph->getMSTShortestDistance() + "\n";
    sendMessage(client_FD, message);

case 7: // Get the average weight of the edges in MST
    data = graph->getMSTAvgEdgeWeight();
    message = "Average weight of the edges in MST: " + std::to_string(data) + "\n";
    break;

case 8: // Total Weight of the MST
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

bool Server::stopClient(int &client_FD)
{ // Stop the client connection
    if (client_FD < 0)
    {
        std::perror("Error: Invalid client file descriptor.");
        return false;
    }
    std::cout << "Client Connection Closed" << std::endl;
    return true;
}

int Server::receiveIntFromClient(int client_FD)
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

std::string Server::receiveStringFromClient(int client_FD)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(client_FD, buffer, sizeof(buffer));
    return std::string(buffer);
}

int main(int argc, char *argv[])
{
    Server *serverObj = pipelineFlag ? new Server(true) : new Server(false);
    delete serverObj;
    return 0;
}