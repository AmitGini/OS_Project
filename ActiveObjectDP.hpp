#ifndef ACTIVEOBJECTDP_HPP
#define ACTIVEOBJECTDP_HPP

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "TaskQueue.hpp"

class PipeDP;

class ActiveObjectDP {
private:
    std::atomic<bool> stop{false}; // Flag to stop the thread
    std::unique_ptr<std::thread> activeObjectThread; // Thread for the active object
    std::unique_ptr<TaskQueue> taskQueue; // Task queue for the active object
    std::shared_ptr<ActiveObjectDP> nextStage; // Pointer to the next stage
    std::mutex activeTask_mutex; // Mutex for the active task
    std::condition_variable activeTask_condition; // Condition variable for the active task
    int stageID; // ID of the stage

    TaskQueue::TaskType currentHandler; // Task handler for the active object
    bool prevStageStatus; // Flag to check if the previous stage is active
    bool working; // Flag to check if the active object is working
    
    void work();

public:
    ActiveObjectDP(int stage);
    ~ActiveObjectDP();
    void enqueue(int& arg1, int arg2);
    void setNextStage(std::shared_ptr<ActiveObjectDP> next);
    void setTaskHandler(TaskQueue::TaskType handler);
    void setPrevStageStatus(bool status);
    void updateNextStage(bool status);
    void notify();
    void makePipeWait(std::mutex &pipeMtx, PipeDP *pipe);
    bool isActive();
};
#endif