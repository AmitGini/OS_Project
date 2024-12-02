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

class LeaderFollowerDP
{
private:
    std::queue<std::weak_ptr<Graph>> queue_taskData;       // Task queue for the active object
    std::vector<std::unique_ptr<std::thread>> threadsPool; // Thread pool that holds the threads
    std::mutex mtx_lf;                                     // Mutex for the threads
    std::condition_variable cv_lf;                         // Condition variable for the threads
    std::atomic<bool> stop{false};                         // Flag to stop the threads

    // Private methods
    void work();            // Enqueues tasks
    void tasksExecution();  // Executes tasks
    void promoteFollower(); // Promotes a follower to leader

public:
    LeaderFollowerDP();
    ~LeaderFollowerDP();

    void processGraphs(std::vector<std::weak_ptr<Graph>> &graphs); // Process the graphs
};

#endif
