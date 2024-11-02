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
    TaskQueue tasksQueue;
    std::mutex activeTask_mutex;
    std::condition_variable activeTask_condition;
    std::unique_ptr<std::thread> activeObjectThread;
    std::atomic<bool> stop{false};
    ActiveObjectDP* nextStage;
    bool prevStageStatus;
    bool working;
    TaskQueue::TaskType currentHandler;
    
    void work();

public:
    ActiveObjectDP();
    ~ActiveObjectDP();
    void enqueue(int& arg1, int arg2);
    void setNextStage(ActiveObjectDP* next);
    void setTaskHandler(TaskQueue::TaskType handler);
    void setPrevStageStatus(bool status);
    void updateNextStage(bool status);
    void notify();
    void makePipeWait(std::mutex &pipeMtx, PipeDP *pipe);
    bool isActive();
};
#endif