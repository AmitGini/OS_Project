#ifndef LEADERFOLLOWER_HPP
#define LEADERFOLLOWER_HPP

#include <iostream>
#include <functional>
#include <queue>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <map>
#include "Graph.hpp"

class LeaderFollower
{
private:
    std::queue<std::weak_ptr<Graph>> queue_taskData;       // Task queue for the active object
    std::queue<std::unique_ptr<std::thread>> threadsPool;  // Queue to manage thread pool and order of promotion to leader
    std::mutex mtx_lf;                                     // Mutex for the threads
    std::condition_variable cv_lf;                         // Condition variable for the threads
    std::atomic<int> const numThreads{4};                  // Number of threads
    std::atomic<bool> stop{false};                         // Flag to stop the threads
    std::atomic<std::thread::id> currentLeader{};  // Track current leader thread
    
    // Private methods
    void work();            // Enqueues tasks
    void executeTask();     // Executes tasks
    void promoteFollower(); // Promotes a follower to leader

public:
    LeaderFollower();
    ~LeaderFollower();

    void processGraphs(std::vector<std::weak_ptr<Graph>> &graphs); // Process the graphs that sended from the server
    void setLeader(std::thread::id id); // Set the leader thread
    bool isLeader(); // Check if the current thread is the leader
};

#endif
