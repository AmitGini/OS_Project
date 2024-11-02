#ifndef LEADERFOLLOWERDP_HPP
#define LEADERFOLLOWERDP_HPP

#include <iostream>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include "FunctionQueue.hpp"
#include "RequestService.hpp"

class LeaderFollowerDP : public RequestService {

private:
    FunctionQueue tasksQueue;
    std::vector<std::thread> threadsPool;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> leaderIndex;
    std::atomic<bool> stop{false};
    int client_fd;

    // Inhertied function to be implemented by derived classes to handle requests
    void handleRequest(int client_FD) override;

public:
    LeaderFollowerDP();
 
    ~LeaderFollowerDP();

    // Promotes a follower to leader
    void promoteFollower(int follower);

    void tasksEnqueing();

    void tasksExecution();
 
    bool enqueingChoices(int choice);

    void enqueueClientTasks(int client_FD);
};

#endif 
