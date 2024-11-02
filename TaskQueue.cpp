#include "TaskQueue.hpp"
#include <iostream>

void TaskQueue::enqueue(TaskType func, int& arg1, int arg2){
    TaskData data{std::move(func), arg1, arg2}; 
    taskQueue.push(std::move(data));
    std::cout << "Task enqueued, task number: " << arg2 << std::endl;
}

bool TaskQueue::executeTask() {
    if (taskQueue.empty()) {
        std::cerr << "Queue is empty!" << std::endl;
        return false;
    }
    
    TaskData data = std::move(taskQueue.front()); 
    taskQueue.pop();
    std::cout<<"Starting Task Execution"<<std::endl;
    return data.task(data.arg1, data.arg2); // Execute the function with stored arguments
}


int TaskQueue::size(){
    return this->taskQueue.size();
}

bool TaskQueue::isEmpty() const{ return taskQueue.empty(); }

void TaskQueue::clear() {
    while (!taskQueue.empty()) {
        taskQueue.pop();
    }
}