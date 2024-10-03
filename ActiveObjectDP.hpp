#ifndef ACTIVEOBJECTDP_HPP
#define ACTIVEOBJECTDP_HPP
#include <iostream>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

static int instanceCount = 0; // Static member variable to count instances

class ActiveObjectDP {
public:
    ActiveObjectDP();
    ~ActiveObjectDP();

    void addTask(std::function<bool()> task);
    bool hasTasks() const;
    bool waitForCompletion();
    bool getMetConditionForNextStage() const;
    void setMetConditionForNextStage(bool status);

private:
    void work();

    std::queue<std::function<bool()>> tasks;
    mutable std::mutex mutex;
    std::condition_variable condition;
    std::thread worker;
    std::atomic<bool> done;

    bool metConditionForNextStage;
    int instanceNumber;  // Member variable to store the unique number for each instance
};

#endif 