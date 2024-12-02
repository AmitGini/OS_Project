#include "ActiveObject.hpp"
#include "Pipeline.hpp"

ActiveObject::ActiveObject(int stage) : stageID(stage), working(false), nextStage(nullptr)
{
    this->queue_taskData = std::queue<std::weak_ptr<Graph>>();                           // task queue for the active object
    this->activeObjectThread = std::make_unique<std::thread>(&ActiveObject::work, this); // Create a new thread for the active object
    printStatus();                                                                       // Printing starting point status
}

ActiveObject::~ActiveObject()
{
    // Join the thread
    if (this->activeObjectThread && this->activeObjectThread->joinable())
    {                                     // Check if the thread is joinable
        this->activeObjectThread->join(); // Join the thread (wait for the thread to finish)
    }
}

// Set the next stage
void ActiveObject::setNextStage(std::weak_ptr<ActiveObject> wptr_nextStage)
{
    if (wptr_nextStage.lock() != nullptr)
    {
        this->nextStage = wptr_nextStage;
    }
    else
    {
        std::cerr << "Set Next Stage Failed, Stage: " << stageID << std::endl;
    }
}

// Set the task handler
void ActiveObject::setTaskHandler(std::function<void(std::weak_ptr<Graph>)> taskFunction)
{
    if (taskFunction)
    {
        std::cout << "Set Task Handler for stage: " << stageID << std::endl;
        this->taskHandler = taskFunction; // Set the task handler to the provided handler
    }
    else
    {
        std::cerr << "Set Task Handler Failed, Stage: " << stageID << std::endl;
    }
}

// Enqueue a task
void ActiveObject::enqueueTask(std::weak_ptr<Graph> graph)
{
    if (graph.lock() != nullptr)
    {
        this->queue_taskData.push(graph);
    }
    else
    {
        std::cerr << "Enqueue Failed, Stage: " << this->stageID << std::endl;
    }
    if (!this->working)
    {
        this->cv_AO.notify_one();
    }
}

// The main work function for the active object
void ActiveObject::work()
{

    // infinite loop till the stop flag is set to true so that the thread can be stopped
    while (!this->stop)
    { // Loop until the stop flag is set

        if (queue_taskData.empty())
        {                          // Check if the task queue is empty
            this->working = false; // Set the working flag to false (in case it was working before it was empty)

            std::unique_lock<std::mutex> lock(this->mtx_AO);
            std::cout << "Stage " << this->stageID << " (Thread " << std::this_thread::get_id() << ") is sleeping" << std::endl;

            // Wait until a task is [enqueued] or [stop flag is set] and [notify condition]
            cv_AO.wait(lock, [this]
                       { return (!this->queue_taskData.empty()) || this->stop; });

            this->working = true; // Set the working flag to true
            std::cout << "Stage " << stageID << " (Thread " << std::this_thread::get_id() << ") has woke up" << std::endl;
            printStatus();
        }

        if (this->stop)
        {
            processStop();
            return;
        }
        else
        {
            std::unique_lock<std::mutex> lock(this->mtx_AO); // Lock the mutex for the active task
            std::weak_ptr<Graph> wptr_graph = this->queue_taskData.front();
            taskHandler(wptr_graph); // taskHandler activate its mst calculation and set it to the graph
            this->queue_taskData.pop();
            if (this->nextStage.lock() != nullptr)
            {
                this->nextStage.lock()->enqueueTask(wptr_graph);
            }
        }
    }
}

void ActiveObject::stopActiveObject()
{
    this->stop = true;
    if (!this->working)
    {
        this->cv_AO.notify_one();
    }
}

void ActiveObject::processStop()
{
    {
        std::unique_lock<std::mutex> lock(mtx_AO); // Lock the mutex
        while (!queue_taskData.empty())
        {
            queue_taskData.front().reset(); // release weak ptr
            queue_taskData.pop();
        }
        activeObjectThread.reset(); // release unique ptr - before destruction join
    }
}

void ActiveObject::printStatus()
{
    std::cout << "Stage " << this->stageID << std::endl;
    int queueSize = this->queue_taskData.size();
    std::cout << "1. Number of Tasks in Queue: " << queueSize << std::endl;
    std::string message = this->working ? "True" : "False";
    std::cout << "2. Working: " << message << std::endl;
}