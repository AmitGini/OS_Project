#include "ActiveObjectDP.hpp"
#include "PipelineDP.hpp"

ActiveObjectDP::ActiveObjectDP() : working(false), prevStageStatus(false), nextStage(nullptr) {
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
    this->currentHandler = handler;
}

void ActiveObjectDP::enqueue(int arg1, int arg2) {
    if (currentHandler) {
        functionQueue.enqueue(currentHandler, arg1, arg2);
    }
}

void ActiveObjectDP::work() {
    bool success = false;
    while (!stop) {
        
        if (functionQueue.isEmpty()) {
            this->working = false;
            this->activeTask_condition.notify_all();

            std::unique_lock<std::mutex> lock(activeTask_mutex);
            std::cout<<"Stage Is Sleeping"<<std::endl;

            activeTask_condition.wait(lock, [this] {
                return (!functionQueue.isEmpty() && this->prevStageStatus) || stop;
            });
            
            std::cout<<"Stage has Woke up"<<std::endl;
        
        } else if(!this->prevStageStatus) {
            success = false;
            this->working = false;
            this->updateNextStage(success);
            std::cout<<" Update Next Stage That I Am Not Ready"<<std::endl;
        
        }else if(!stop){
            std::unique_lock<std::mutex> lock(activeTask_mutex);
            this->working = true;
            success = functionQueue.dequeueAndExecute();
            if (success) {
                this->updateNextStage(success);
                std::cout<<"Has Stage Executed? "<< success <<std::endl;
            }

        } else return;  // Stop the thread
    }
}

void ActiveObjectDP::setPrevStageStatus(bool status) {
    this->prevStageStatus = status;
}

void ActiveObjectDP::updateNextStage(bool status) {
    if (!nextStage) return;
    this->nextStage->setPrevStageStatus(status);
    if (status) nextStage->notify();
}

void ActiveObjectDP::notify() {
    if (functionQueue.isEmpty() || this->working || !this->prevStageStatus) {
        std::cout<<"Didnt Notify Stage"<<std::endl;
        std::cout<<"Function Queue Empty: "<<functionQueue.isEmpty()<<std::endl;
        std::cout<<"Working: "<<this->working<<std::endl;
        std::cout<<"Prev Stage Status: "<<this->prevStageStatus<<std::endl;
        return;
    }

    this->working = true;
    std::cout<<"Notified Stage Executed"<<std::endl;
    this->activeTask_condition.notify_all();
}

bool ActiveObjectDP::isWorking() {
    return this->working;
}

void ActiveObjectDP::makePipeWait(std::mutex &pipeMtx, PipelineDP *pipe) {
    if (this->working) {
    std::unique_lock<std::mutex> lock_pipe(pipeMtx);
    std::cout<<"Stage is Working"<<std::endl;
    pipe->setStageWorkStatus(true);
    activeTask_condition.wait(lock_pipe, [this] { return !this->isWorking(); });
    pipe->setStageWorkStatus(false);
    std::cout<<"A Stage Finished its work"<<std::endl;
    } else {
        pipe->setStageWorkStatus(false);
        std::cout<<"Stage is not working"<<std::endl;
    }
}
