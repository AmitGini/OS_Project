#ifndef LEADERFOLLOWERDP_HPP
#define LEADERFOLLOWERDP_HPP

#include <iostream>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include "TaskQueue.hpp"
#include "RequestService.hpp"

class LeaderFollowerDP : public RequestService {

private:
    TaskQueue tasksQueue;
    std::vector<std::thread> threadsPool;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> leaderIndex;
    std::atomic<bool> stop{false};
    int client_fd;

    // Inhertied function to be implemented by derived classes to handle requests

public:
    LeaderFollowerDP();
    ~LeaderFollowerDP();
    void handleRequest(int& client_FD) override;
    void promoteFollower(int follower);  // Promotes a follower to leader
    void tasksEnqueing();
    void tasksExecution();
    bool enqueingChoices(int choice);
};

#endif 
