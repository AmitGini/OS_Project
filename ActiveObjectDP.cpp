#include "ActiveObjectDP.hpp"
#include "PipeDP.hpp"

ActiveObjectDP::ActiveObjectDP() : working(false), prevStageStatus(false), nextStage(nullptr) {
    activeObjectThread = std::make_unique<std::thread>(&ActiveObjectDP::work, this);
}

ActiveObjectDP::~ActiveObjectDP() {
    {
        std::unique_lock<std::mutex> lock(activeTask_mutex);
        stop = true;
    }
    // Clear the queue and notify the thread
    tasksQueue.clear();
    activeTask_condition.notify_all();

    // Join the thread
    if (activeObjectThread && activeObjectThread->joinable()) {
        activeObjectThread->join();
    }
}

void ActiveObjectDP::setNextStage(ActiveObjectDP* next) {
    this->nextStage = next;
    if(nextStage == this) this->prevStageStatus = true;
}

void ActiveObjectDP::setTaskHandler(TaskQueue::TaskType handler) {
    this->currentHandler = handler;
}

void ActiveObjectDP::enqueue(int& arg1, int arg2) {
    if (currentHandler) {
        tasksQueue.enqueue(currentHandler, arg1, arg2);
        notify();
    }
}

void ActiveObjectDP::work() {
    bool success = false;
    while (!stop) {
        
        if (tasksQueue.isEmpty()) {
            this->working = false;
            this->activeTask_condition.notify_all();

            std::unique_lock<std::mutex> lock(activeTask_mutex);
            std::cout<<"Stage Is Sleeping"<<std::endl;

            activeTask_condition.wait(lock, [this] {
                return (!tasksQueue.isEmpty() && this->prevStageStatus) || stop;
            });
            
            std::cout<<"Stage has Woke up"<<std::endl;
        
        } else if(!this->prevStageStatus) {
            success = false;
            this->working = false;
            this->updateNextStage(success);
        
        }else if(!stop){
            std::unique_lock<std::mutex> lock(activeTask_mutex);
            this->working = true;
            success = tasksQueue.executeTask();
            if (success) {
                this->updateNextStage(success);
            }

        } else return;  // Stop the thread
    }
}

void ActiveObjectDP::setPrevStageStatus(bool status) {
    if(this->nextStage && this->nextStage == this) return;  // The exit stage it its own next stage
    this->prevStageStatus = status;  // My next stage ask my status (I am the previous stage of my Next Stage)
}

void ActiveObjectDP::updateNextStage(bool status) {
    if (this->nextStage && this->nextStage == this) return;
    this->nextStage->setPrevStageStatus(status);
    std::cout<<"******** Notifying Next Stage ********"<<std::endl;    
    nextStage->notify();
}

void ActiveObjectDP::notify() {
    if(this->nextStage && this->nextStage == this && tasksQueue.isEmpty()){
        std::cout<<"****** Client has Disconnected ******"<<std::endl;
    }
    else if (tasksQueue.isEmpty() || this->working || !this->prevStageStatus) {
        std::cout<<"**** Failed to Notify The Stage Due: ****"<<std::endl;
        int queueSize = tasksQueue.size();
        std::cout<<"1. Number of Tasks in Queue: "<<queueSize<<std::endl;
        std::string message = this->working ? "True" : "False";
        std::cout<<"2. Working: "<<message<<std::endl;
        message = this->prevStageStatus ? "True" : "False";
        std::cout<<"3. Prev Stage Completed: "<<message<<std::endl;
    }else{
        this->working = true;
        std::cout<<"Notify Stage"<<std::endl;
        this->activeTask_condition.notify_all();
        sleep(0.5);  // Sleep for 1 second
    }
}

bool ActiveObjectDP::isActive() {
    return this->working;
}

void ActiveObjectDP::makePipeWait(std::mutex &pipeMtx, PipeDP *pipe) {
    if (this->working) {

    std::unique_lock<std::mutex> lock_pipe(pipeMtx);
    std::cout<<"Stage is Working"<<std::endl;
    pipe->setStageActiveStatus(true);

    activeTask_condition.wait(lock_pipe, [this] { return !this->isActive(); });
    pipe->setStageActiveStatus(false);

    std::cout<<"Stage Finished its work"<<std::endl;
    } else {
        pipe->setStageActiveStatus(false);
        std::cout<<"Stage is not working"<<std::endl;
    }
}
