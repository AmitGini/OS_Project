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
    std::atomic<bool> stop{false};
    std::unique_ptr<std::thread> activeObjectThread;
    std::unique_ptr<TaskQueue> taskQueue;
    std::shared_ptr<ActiveObjectDP> nextStage;
    std::mutex activeTask_mutex;
    std::condition_variable activeTask_condition;

    TaskQueue::TaskType currentHandler;
    bool prevStageStatus;
    bool working;
    
    void work();

public:
    ActiveObjectDP();
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