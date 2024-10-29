#ifndef FUNCTIONQUEUE_HPP
#define FUNCTIONQUEUE_HPP

#include <functional>
#include <queue>
#include <utility>
#include <iostream>

class FunctionQueue {
public:
    using FuncType = std::function<bool(int, int)>;
    
    // Enqueue a function with its arguments into the queue
    void enqueue(FuncType func, int arg1, int arg2);
    
    // Dequeue a function and its arguments from the queue
    std::pair<FuncType, std::pair<int, int>> dequeue();
    
    // Dequeue and immediately execute a function with its stored arguments
    bool dequeueAndExecute();
    
    // Check if queue is empty
    bool isEmpty() const ;

    void clear();
private:
    // Store both the function and its arguments
    struct FunctionData {
        FuncType func;
        int arg1;
        int arg2;
    };
    std::queue<FunctionData> funcQueue; // Queue to store the functions with their arguments
};

#endif 