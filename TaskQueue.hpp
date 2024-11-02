#ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include <functional>
#include <queue>
#include <utility>
#include <iostream>

class TaskQueue {
public:
    using TaskType = std::function<bool(int&, int)>;
    void enqueue(TaskType task, int& arg1, int arg2);  // Enqueue a function with its arguments into the queue
    bool executeTask();  // Dequeue and immediately execute a function with its stored arguments
    bool isEmpty() const;   // Check if queue is empty
    int size();
    void clear();  // Clear the queue

private:
    // Store both the function and its arguments
    struct TaskData {
        TaskType task;
        int& arg1;
        int arg2;
    };

    std::queue<TaskData> taskQueue; // Queue to store the functions with their arguments
};

#endif 