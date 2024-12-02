#ifndef ACTIVEOBJECT_HPP
#define ACTIVEOBJECT_HPP

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <queue>
#include <functional>
#include <utility>
#include "TaskQueue.hpp"
#include "Graph.hpp"

class ActiveObject
{
private:
    std::function<void(std::weak_ptr<Graph>)> taskHandler; // Task handler for the active object
    std::queue<std::weak_ptr<Graph>> queue_taskData;       // Task queue for the active object
    std::unique_ptr<std::thread> activeObjectThread;       // Thread for the active object
    std::weak_ptr<ActiveObject> nextStage;                 // Pointer to the next stage
    std::mutex mtx_AO;                                     // Mutex for the active task
    std::condition_variable cv_AO;                         // Condition variable for the active task
    std::atomic<bool> stop{false};                         // Flag to stop the thread
    int stageID;                                           // ID of the stage
    bool working;                                          // Flag to check if the active object is working

    void work();        // Work function for the active object
    void processStop(); // After stop flag detected - initial process to stop the active object before destruction
    void printStatus(); // Print the status of the active object

public:
    ActiveObject(int stage);                                                     // Constructor
    ~ActiveObject();                                                             // Destructor
    void enqueueTask(std::weak_ptr<Graph> wptr_graph);                           // Enqueue a graph to the task queue
    void setNextStage(std::weak_ptr<ActiveObject> wptr_nextStage);               // Set the next stage
    void setTaskHandler(std::function<void(std::weak_ptr<Graph>)> taskFunction); // Set the task handler
    void stopActiveObject();                                                     // Stop the active object
};
#endif