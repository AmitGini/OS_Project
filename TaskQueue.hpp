#ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include <queue>
#include <memory>
#include <iostream>
#include <functional>

class TaskQueue {
public:
    using TaskType = std::function<bool(int&, int)>;
    ~TaskQueue();
    void enqueue(TaskType task, int& arg1, int arg2);  // Enqueue a function with its arguments into the queue
    bool executeTask();  // Dequeue and immediately execute a function with its stored arguments
    bool isEmpty() const;   // Check if queue is empty
    int size();
    void clearQueue();  // Clear the queue
private:
    // Store both the function and its arguments
    struct TaskData {
            TaskType task;
            int arg1;
            int arg2;
    };

    std::queue<std::unique_ptr<TaskData>> taskQueue;  // Queue to store the functions with their arguments
};

#endif 