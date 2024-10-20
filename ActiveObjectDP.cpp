#include "ActiveObjectDP.hpp"

ActiveObjectDP::ActiveObjectDP()
    : working(false),
      prevStageStatus(false),
      nextStage(nullptr)
{
    activeObjectThread = std::make_unique<std::thread>(&ActiveObjectDP::work, this);
}

ActiveObjectDP::~ActiveObjectDP() {
    {
        std::unique_lock<std::mutex> lock(activeTask_mutex);
        stop = true;
    }
    activeTask_condition.notify_one();
    if (activeObjectThread && activeObjectThread->joinable()) {
        activeObjectThread->join();
    }
}

void ActiveObjectDP::setNextStage(ActiveObjectDP* next) {
    this->nextStage = next;
}

void ActiveObjectDP::setTaskHandler(FunctionQueue::FuncType handler) {
    std::lock_guard<std::mutex> lock(activeTask_mutex);
    this->currentHandler = handler;
}

void ActiveObjectDP::enqueue(int arg1, int arg2) {
    std::lock_guard<std::mutex> lock(activeTask_mutex);
    if (currentHandler) {
        functionQueue.enqueue(currentHandler, arg1, arg2);
    }
}

void ActiveObjectDP::work() {
    bool success = false;
    while (!stop) {
        {
            std::unique_lock<std::mutex> lock(activeTask_mutex);
            this->working = false;
            if (!functionQueue.isEmpty()) {
                if (!this->prevStageStatus) {
                    success = false;
                }
            }
            activeTask_condition.wait(lock, [this] {
                return (!functionQueue.isEmpty() && this->prevStageStatus) || stop;
            });
            
            if (stop) return;
            this->working = true;
        }
        
        if (!functionQueue.isEmpty()) {
            std::lock_guard<std::mutex> lock(activeTask_mutex);
            success = functionQueue.dequeueAndExecute();
            
            if (success && functionQueue.isEmpty()) {
                this->updateNextStage(success);
            }
        }
    }
}

void ActiveObjectDP::setPrevStageStatus(bool status) {
    this->prevStageStatus = status;
}

void ActiveObjectDP::updateNextStage(bool status) {
    if (!nextStage) return;
    nextStage->setPrevStageStatus(status);
    if (status) nextStage->notify();
}

void ActiveObjectDP::notify() {
    if (functionQueue.isEmpty() || this->working || !this->prevStageStatus) return;
    this->activeTask_condition.notify_one();
}

bool ActiveObjectDP::isWorking() {
    return this->working;
}