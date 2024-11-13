#include "ActiveObjectDP.hpp"
#include "PipeDP.hpp"

ActiveObjectDP::ActiveObjectDP(int stage) : stageID(stage), working(false), prevStageStatus(false), nextStage(nullptr) {
    taskQueue = std::make_unique<TaskQueue>(); // Create a new task queue for the active object
    activeObjectThread = std::make_unique<std::thread>(&ActiveObjectDP::work, this); // Create a new thread for the active object
}

ActiveObjectDP::~ActiveObjectDP() {
    {
        std::unique_lock<std::mutex> lock(activeTask_mutex); // Lock the mutex
        stop = true; // Set the stop flag to true to stop the thread
    }

    // Clear the queue and notify the thread
    this->taskQueue->clear();
    this->activeTask_condition.notify_all(); // notify all the threads waiting on the condition variable to check the stop flag

    // Join the thread
    if (this->activeObjectThread && this->activeObjectThread->joinable()) { // Check if the thread is joinable 
        this->activeObjectThread->join(); // Join the thread (wait for the thread to finish)
    }
}

// Set the next stage
void ActiveObjectDP::setNextStage(std::shared_ptr<ActiveObjectDP> next) {
    this->nextStage = next;
    if(nextStage.get() == this) this->prevStageStatus = true; // If the next stage is the same as this stage, set the previous stage status to true
}

// Set the task handler
void ActiveObjectDP::setTaskHandler(TaskQueue::TaskType handler) {
    this->currentHandler = handler; // Set the task handler to the provided handler
}

// Enqueue a task
void ActiveObjectDP::enqueue(int& arg1, int arg2) {
    if (this->currentHandler) { // Check if the task handler is set
        this->taskQueue->enqueue(this->currentHandler, arg1, arg2); // Enqueue the task with the provided arguments
        notify(); // Notify the thread to start working
    }
}

// The main work function for the active object
void ActiveObjectDP::work() {
    bool success = false; // Flag to check if the task was successful, set to false by default
    
    // infinite loop till the stop flag is set to true so that the thread can be stopped
    while (!this->stop) { // Loop until the stop flag is set
        
        if (taskQueue->isEmpty()) { // Check if the task queue is empty
            this->working = false; // Set the working flag to false
            this->activeTask_condition.notify_all(); // Notify all the threads waiting on the condition variable

            std::unique_lock<std::mutex> lock(activeTask_mutex);
             std::cout << "Stage " << stageID << " (Thread " << std::this_thread::get_id() << ") is sleeping" << std::endl;
            
            // Wait until a task is enqueued and the previous stage is done working or the stop flag is set
            activeTask_condition.wait(lock, [this] {
                return (!taskQueue->isEmpty() && this->prevStageStatus) || stop;
            });
            
            std::cout << "Stage " << stageID << " (Thread " << std::this_thread::get_id() << ") has woke up" << std::endl;
        
        } else if(!this->prevStageStatus) { // Check if the previous stage is active
            success = false; // Set the success flag to false
            this->working = false; // Set the working flag to false so that the stage needs to wait
            this->updateNextStage(success); // Update the next stage with the success flag (false) to stop the next stage
        
        }else if(!stop){ // Check if the stop flag is not set
            std::unique_lock<std::mutex> lock(this->activeTask_mutex); // Lock the mutex for the active task
            this->working = true; // Set the working flag to true
            success = taskQueue->executeTask(); // Execute the task and store the result in the success flag
            if (success) { // Check if the task was successful 
                this->updateNextStage(success); // Update the next stage with the success flag
            }

        } else return;  // If the stop flag is set, return to stop the thread
    }
}

// Set the previous stage status
void ActiveObjectDP::setPrevStageStatus(bool status) {
    if(this->nextStage && this->nextStage.get() == this) return;  // If the next stage is the same as this stage, return because the next stage is not set (exit condition)
    this->prevStageStatus = status;  // Set the previous stage status to the provided status
}

// Update the next stage
void ActiveObjectDP::updateNextStage(bool status) {
    if (this->nextStage && this->nextStage.get() == this) return; // If the next stage is the same as this stage, return because the next stage is not set (exit condition)
    this->nextStage->setPrevStageStatus(status); // Set the previous stage status of the next stage to the provided status (this stage's success status)
    std::cout<<"******** Notifying Next Stage (Stage number: "<< this->nextStage->stageID <<") ********"<<std::endl;    
    if (this->stageID == 4) { // Check if the next stage is the client exit stage
        std::cout<<"****** Client has Disconnected ******"<<std::endl;
    } else {
    this->nextStage->notify(); // Notify the next stage
    }
}

// Notify the stage
void ActiveObjectDP::notify() {
    // Check if the task queue is empty, the stage is working, or the previous stage is not active
    if (taskQueue->isEmpty() || this->working || !this->prevStageStatus) { 
        if (this->prevStageStatus){
            std::cout<<"**** Status of stage "<< stageID << " (Theard " << std::this_thread::get_id() <<") "<<std::endl;
        } else {
            std::cout<<"**** Failed to Notify The Stage "<< stageID << " (Thread " << std::this_thread::get_id() <<" Due: ****"<<std::endl;
        }
        int queueSize = taskQueue->size();
        std::cout<<"1. Number of Tasks in Queue: "<<queueSize<<std::endl;
        std::string message = this->working ? "True" : "False";
        std::cout<<"2. Working: "<<message<<std::endl;
        message = this->prevStageStatus ? "True" : "False";
        std::cout<<"3. Prev Stage Completed: "<<message<<std::endl;
    }else{
        this->working = true;
        std::cout<<"Notify Stage " << stageID <<std::endl;
        this->activeTask_condition.notify_all(); // Notify all the threads waiting on the condition variable to start working
        sleep(0.5);  // Sleep for 0.5 seconds to allow the thread to start working
    }
}

bool ActiveObjectDP::isActive() {
    return this->working;
}

// Make the pipe wait for the stage
void ActiveObjectDP::makePipeWait(std::mutex &pipeMtx, PipeDP *pipe) {
    if (this->working) { // Check if the stage is working

    std::unique_lock<std::mutex> lock_pipe(pipeMtx); // Lock the pipe mutex
    std::cout<<"Stage is Working"<<std::endl;
    pipe->setStageActiveStatus(true);  // Set the stage active status to true

    activeTask_condition.wait(lock_pipe, [this] { return !this->isActive(); }); // Wait until the stage is not active
    pipe->setStageActiveStatus(false); // Set the stage active status to false

    std::cout<<"Stage Finished its work"<<std::endl;
    } else { // If the stage is not working
        pipe->setStageActiveStatus(false); // Set the stage active status to false
        std::cout<<"Stage is not working"<<std::endl;
    }
}
