#include "ActiveObjectDP.hpp"
#include "PipeDP.hpp"

ActiveObjectDP::ActiveObjectDP() : working(false), prevStageStatus(false), nextStage(nullptr) {
    this->taskQueue = std::make_unique<TaskQueue>();
    this->activeObjectThread = std::make_unique<std::thread>(&ActiveObjectDP::work, this);
    this->activeObjectThread->detach();
}

ActiveObjectDP::~ActiveObjectDP() {
    // Join the thread
    std::cout<<"**** Deleting Active Object ****"<<std::endl;
    if (this->activeObjectThread && this->activeObjectThread->joinable()) {
        this->activeObjectThread->join();
    }

    std::cout<<"Active-Object: Thread Joined"<<std::endl;
    if(this->taskQueue) {
        this->taskQueue->clearQueue();
        this->taskQueue.reset();
        std::cout<<"Active-Object: Cleared Task Queue"<<std::endl;
    }
    if(this->nextStage){
        this->nextStage->clearActiveObject();
        this->nextStage.reset();
        std::cout<<"Active-Object: Cleared Next Stage"<<std::endl;
    }
    if(this->activeObjectThread){
        this->activeObjectThread.reset();
        std::cout<<"Active-Object: Reset Thread"<<std::endl;
    }
    std::cout<<"**** Active Object: Done ****"<<std::endl;
}

void ActiveObjectDP::setNextStage(std::shared_ptr<ActiveObjectDP> next) {
    this->nextStage = next;
    if(this->nextStage.get() == this) this->prevStageStatus = true;
}

void ActiveObjectDP::setTaskHandler(TaskQueue::TaskType handler) {
    this->currentHandler = handler;
}

void ActiveObjectDP::enqueue(int& arg1, int arg2) {
    if (this->currentHandler) {
        this->taskQueue->enqueue(this->currentHandler, arg1, arg2);
        notify();
    }
}

void ActiveObjectDP::work() {
    bool success = false;
    while (!this->stop) {
        
        if (taskQueue->isEmpty()) {
            this->working = false;
            this->activeTask_condition.notify_all();

            std::unique_lock<std::mutex> lock(activeTask_mutex);
            std::cout<<"Stage Is Sleeping"<<std::endl;

            this->activeTask_condition.wait(lock, [this] {
                return (!taskQueue->isEmpty() && this->prevStageStatus) || this->stop;
            });
            std::cout<<"Stage has Woke up"<<std::endl;
            
        }else if(this->stop){
            std::cout<<"Stage is Stopped"<<std::endl;
            return;
        } else if(!this->prevStageStatus) {
            success = false;
            this->working = false;
            this->updateNextStage(success);
        }else {
            std::unique_lock<std::mutex> lock(this->activeTask_mutex);
            this->working = true;
            success = taskQueue->executeTask();
            if (success) {
                this->updateNextStage(success);
            }
        }
    }
}

void ActiveObjectDP::setPrevStageStatus(bool status) {
    if(this->nextStage && this->nextStage.get() == this) return;  // The exit stage it its own next stage
    this->prevStageStatus = status;  // My next stage ask my status (I am the previous stage of my Next Stage)
}

void ActiveObjectDP::updateNextStage(bool status) {
    if (this->nextStage && this->nextStage.get() == this) return;
    this->nextStage->setPrevStageStatus(status);
    std::cout<<"******** Notifying Next Stage ********"<<std::endl;    
    this->nextStage->notify();
}

void ActiveObjectDP::notify() {
    if(this->nextStage && this->nextStage.get() == this && taskQueue->isEmpty()){
        std::cout<<"****** Client has Disconnected ******"<<std::endl;
    }
    else if (this->taskQueue->isEmpty() || this->working || !this->prevStageStatus) {
        std::cout<<"**** Failed to Notify The Stage Due: ****"<<std::endl;
        int queueSize = taskQueue->size();
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

    activeTask_condition.wait(lock_pipe, [this] { return !this->isActive();});
    pipe->setStageActiveStatus(false);

    std::cout<<"Stage Finished its work"<<std::endl;
    } else {
        pipe->setStageActiveStatus(false);
        std::cout<<"Stage is not working"<<std::endl;
    }
}

void ActiveObjectDP::clearActiveObject(){
    {
        std::unique_lock<std::mutex> lock(this->activeTask_mutex);
        this->stop = true;
    }
    std::cout<<"Active-Object: Notifying Stage Stop Flag"<<std::endl;
    this->activeTask_condition.notify_all();
}
