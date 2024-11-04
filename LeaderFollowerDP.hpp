#ifndef LEADERFOLLOWERDP_HPP
#define LEADERFOLLOWERDP_HPP

#include <iostream>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <optional>
#include <atomic>
#include <vector>
#include <memory>
#include "TaskQueue.hpp"
#include "RequestService.hpp"

class LeaderFollowerDP : public RequestService {

private:
    // Constants
    static constexpr int NO_LEADER = -1;
    static constexpr int CONVERSATION = 0;
    static constexpr int TASKS = 1;

    // Thread managment
    std::vector<std::unique_ptr<std::thread>> threadsPool;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> leaderIndex;
    std::atomic<bool> stop{false};
    std::atomic<bool> toCloseClient{false};

    // Task managment
    std::unique_ptr<TaskQueue> tasksQueue;
    int client_fd;

    // Private methods
    void tasksEnqueing();
    void tasksExecution();
    bool enqueingChoices(int choice);
    void promoteFollower(int follower);  // Promotes a follower to leader
    // void handleError(const std::string& error_msg);
    // void cleanupResources();
    
public:
    LeaderFollowerDP();
    ~LeaderFollowerDP();
    void handleRequest(int& client_FD) override;

};

#endif 
