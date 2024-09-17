#ifndef SERVER_HPP
#define SERVER_HPP

#include "Graph.hpp"
#include "MSTStrategy.hpp"
#include <pthread.h>
#include <queue>
#include <condition_variable>
#include <functional>
#include <memory>

class Server
{
public:
    Server(int port, int numThreads);
    ~Server();
    void start();
    void handleRequest(int clientSocket);
    void addTask(const std::function<void()> &task);
    void workerThread();

private:
    int port;
    int numThreads;
    int serverSocket;
    Graph graph;
    std::unique_ptr<MSTStrategy> mstStrategy;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    std::queue<std::function<void()>> taskQueue;
    std::vector<pthread_t> threads;
    bool stop;
    pthread_t leaderThread;
    pthread_mutex_t leaderMtx;
    std::queue<pthread_t> workerQueue;

    void processRequest(const std::string &request, std::string &response, int clientSocket);
    void runPipeline();
    void runLeaderFollower();
    void leaderThreadFunc();
    void designateNewLeader();
};

#endif
