#ifndef LEADERFOLLOWERDP_HPP
#define LEADERFOLLOWERDP_HPP

#include <iostream>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <optional> // for std::optional 
#include <atomic>
#include "TaskQueue.hpp"
#include "RequestService.hpp"
#include <map>

class LeaderFollowerDP : public RequestService {

private:
    // Constants
    static constexpr int NO_LEADER = -1; // No leader
    static constexpr int CONVERSATION = 0; // Conversation with client
    static constexpr int TASKS = 1; // Tasks execution 

    // Thread managment
    std::vector<std::thread> threadsPool; // Thread pool that holds the threads
    std::mutex mtx; // Mutex for the threads
    std::condition_variable cv; // Condition variable for the threads
    std::atomic<int> leaderIndex; // Index of the leader thread
    std::atomic<bool> stop{false}; // Flag to stop the threads
    std::atomic<bool> toCloseClient{false}; // Flag to close the client

    // Task managment
    TaskQueue tasksQueue;
    int client_fd; // Client file descriptor


    // Private methods
    void tasksEnqueing(); // Enqueues tasks
    void tasksExecution(); // Executes tasks
    bool enqueingChoices(int choice); // manages the enqueing of choices 
    void promoteFollower(int follower);  // Promotes a follower to leader
    // void handleError(const std::string& error_msg);
    // void cleanupResources();
    
public:
    LeaderFollowerDP();
    ~LeaderFollowerDP();
    void handleRequest(int& client_FD) override;

};

#endif 
