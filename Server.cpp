#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

Server::Server(int port, int numThreads)
    : port(port), numThreads(numThreads), graph(10), stop(false)
{
    mstStrategy = MSTFactory::createMSTAlgorithm(MSTFactory::PRIM); // default to Prim

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        close(serverSocket);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, 5) < 0)
    {
        close(serverSocket);
        throw std::runtime_error("Failed to listen on socket");
    }

    pthread_mutex_init(&mtx, nullptr);
    pthread_mutex_init(&leaderMtx, nullptr);
    pthread_cond_init(&cv, nullptr);
}

Server::~Server()
{
    stop = true;
    pthread_cond_broadcast(&cv);
    for (pthread_t &thread : threads)
    {
        pthread_join(thread, nullptr);
    }
    close(serverSocket);
    pthread_mutex_destroy(&mtx);
    pthread_mutex_destroy(&leaderMtx);
    pthread_cond_destroy(&cv);
}

void Server::start()
{
    for (int i = 0; i < numThreads; ++i)
    {
        pthread_t thread;
        pthread_create(&thread, nullptr, [](void *arg) -> void *
                       {
            static_cast<Server*>(arg)->workerThread();
            return nullptr; }, this);
        threads.push_back(thread);
        workerQueue.push(thread);
    }

    pthread_create(&leaderThread, nullptr, [](void *arg) -> void *
                   {
        static_cast<Server*>(arg)->leaderThreadFunc();
        return nullptr; }, this);
    runLeaderFollower();
}

void Server::handleRequest(int clientSocket)
{
    char buffer[1024];
    int len = read(clientSocket, buffer, sizeof(buffer));
    if (len < 0)
    {
        close(clientSocket);
        return;
    }

    std::string request(buffer, len);
    std::string response;
    processRequest(request, response, clientSocket);
    if (!response.empty())
    {
        write(clientSocket, response.c_str(), response.size());
    }
    close(clientSocket);
}

void Server::addTask(const std::function<void()> &task)
{
    {
        pthread_mutex_lock(&mtx);
        taskQueue.push(task);
        pthread_mutex_unlock(&mtx);
    }
    pthread_cond_signal(&cv);
}

void Server::workerThread()
{
    while (true)
    {
        std::function<void()> task;
        {
            pthread_mutex_lock(&mtx);
            while (taskQueue.empty() && !stop)
            {
                pthread_cond_wait(&cv, &mtx);
            }
            if (stop && taskQueue.empty())
            {
                pthread_mutex_unlock(&mtx);
                return;
            }
            task = taskQueue.front();
            taskQueue.pop();
            pthread_mutex_unlock(&mtx);
        }
        task();
    }
}

void Server::processRequest(const std::string &request, std::string &response, int clientSocket)
{
    pthread_mutex_lock(&mtx);
    if (request == "Prim")
    {
        mstStrategy = MSTFactory::createMSTAlgorithm(MSTFactory::PRIM);
    }
    else if (request == "Kruskal")
    {
        mstStrategy = MSTFactory::createMSTAlgorithm(MSTFactory::KRUSKAL);
    }
    else if (request == "TotalWeight")
    {
        response = "TotalWeight: " + std::to_string(mstStrategy->getTotalWeight());
    }
    else if (request == "LongestDist")
    {
        response = "LongestDist: " + std::to_string(mstStrategy->getLongestDistance());
    }
    else if (request == "ShortestDist")
    {
        response = "ShortestDist: " + std::to_string(mstStrategy->getShortestDistance());
    }
    else if (request == "AvgDist")
    {
        response = "AvgDist: " + std::to_string(mstStrategy->getAverageDistance());
    }
    else if (request == "Newgraph")
    {
        // Implement new graph logic
    }
    else if (request == "NewEdge")
    {
        // Implement new edge logic
    }
    else if (request == "RemoveEdge")
    {
        // Implement remove edge logic
    }
    else if (request == "Exit")
    {
        // Just close the client's connection
        response = "Client exiting";
        close(clientSocket);
        pthread_mutex_unlock(&mtx);
        return;
    }
    else
    {
        response = "Invalid request";
    }
    mstStrategy->computeMST(graph);
    response += "\nMST computed";
    pthread_mutex_unlock(&mtx);
}

void Server::runPipeline()
{
    while (true)
    {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0)
        {
            continue;
        }

        addTask([this, clientSocket]
                { handleRequest(clientSocket); });
    }
}

void Server::runLeaderFollower()
{
    while (true)
    {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0)
        {
            continue;
        }

        handleRequest(clientSocket);
    }
}

void Server::leaderThreadFunc()
{
    while (!stop)
    {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0)
        {
            continue;
        }

        handleRequest(clientSocket);
        designateNewLeader();
    }
}

void Server::designateNewLeader()
{
    pthread_mutex_lock(&leaderMtx);
    if (!workerQueue.empty())
    {
        leaderThread = workerQueue.front();
        workerQueue.pop();
        workerQueue.push(leaderThread);
    }
    pthread_mutex_unlock(&leaderMtx);
}
