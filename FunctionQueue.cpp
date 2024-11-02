#include "FunctionQueue.hpp"
#include <iostream>

void FunctionQueue::enqueue(FuncType func, int arg1, int arg2){
    FunctionData data{std::move(func), arg1, arg2}; 
    funcQueue.push(std::move(data));
    std::cout << "Function enqueued, task number: " << arg2 << std::endl;
}

std::pair<FunctionQueue::FuncType, std::pair<int, int>> FunctionQueue::dequeue() {
    if (funcQueue.empty()) {
        throw std::runtime_error("Queue is empty, cannot dequeue.");
    }
    
    FunctionData data = funcQueue.front();
    funcQueue.pop();
    return {data.func, {data.arg1, data.arg2}};
}

bool FunctionQueue::dequeueAndExecute() {
    if (funcQueue.empty()) {
        std::cerr << "Queue is empty!" << std::endl;
        return false;
    }
    
    FunctionData data = std::move(funcQueue.front()); 
    funcQueue.pop();
    std::cout<<"Function dequeued and executed"<<std::endl;
    return data.func(data.arg1, data.arg2); // Execute the function with stored arguments
}

void FunctionQueue::removeTasksByClient(int client_fd) {
    std::queue<FunctionData> tempQueue;

    while (!funcQueue.empty()) {
        FunctionData data = std::move(funcQueue.front());
        funcQueue.pop();
        
        if (data.arg1 != client_fd && data.arg1 > 0) {
            tempQueue.push(std::move(data));
        }
    }

    funcQueue = std::move(tempQueue);
}

bool FunctionQueue::isEmpty() const{ return funcQueue.empty(); }

void FunctionQueue::clear() {
    while (!funcQueue.empty()) {
        funcQueue.pop();
    }
}