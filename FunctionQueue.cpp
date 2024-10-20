#include "FunctionQueue.hpp"
#include <iostream>

void FunctionQueue::enqueue(FuncType func, int arg1, int arg2) {
    FunctionData data{func, arg1, arg2};
    funcQueue.push(data);
    std::cout<<"Function enqueued, task number:"<<arg2<<std::endl;
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
    
    FunctionData data = funcQueue.front();
    funcQueue.pop();
    return data.func(data.arg1, data.arg2); // Execute the function with stored arguments
}