#include "TaskQueue.hpp"

TaskQueue::~TaskQueue(){
    clearQueue();
}

void TaskQueue::enqueue(TaskType func, int &arg1, int arg2){
    std::unique_ptr<TaskData> data = std::make_unique<TaskData>(TaskData{
        std::move(func), 
        arg1,
        arg2
    });
    this->taskQueue.push(std::move(data));
    std::cout << "Task enqueued, task number: " << arg2 << std::endl;
}

bool TaskQueue::executeTask() {
    if (this->taskQueue.empty()) {
        std::cerr << "Queue is empty!" << std::endl;
        return false;
    }
    
    std::unique_ptr<TaskData> data = std::move(this->taskQueue.front()); 
    this->taskQueue.pop();
    std::cout<<"Starting Task Execution"<<std::endl;
    return data->task(data->arg1, data->arg2); // Execute the function with stored arguments
}

int TaskQueue::size(){
    return this->taskQueue.size();
}

bool TaskQueue::isEmpty() const { 
    return this->taskQueue.empty();
}

void TaskQueue::clearQueue() {
    while (!this->taskQueue.empty()) {
        std::unique_ptr<TaskData> data = std::move(this->taskQueue.front());
        this->taskQueue.pop();
        data.reset();
    }
    std::cout << "Queue cleared!" << std::endl;
}