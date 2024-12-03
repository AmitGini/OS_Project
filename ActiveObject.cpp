#include "ActiveObject.hpp"

ActiveObject::ActiveObject(int stage) : stageID(stage), working(false), stop(false)
{
    this->queue_taskData = std::queue<std::weak_ptr<Graph>>();                           // task queue for the active object
    this->activeObjectThread = std::make_unique<std::thread>(&ActiveObject::work, this); // Create a new thread for the active object
    printStatus();                                                                       // Printing starting point status
}

ActiveObject::~ActiveObject()
{
    // // Check if the thread is joinable
    std::cout << "\nActive-Object - Stage " << stageID << " : Destruction Activated" << std::endl;

    if (this->activeObjectThread && this->activeObjectThread->joinable())
    {
        std::cout << "\nActive-Object - Stage " << stageID << ": Join Thread - Destruction" << std::endl;
        this->activeObjectThread->join(); // Join the thread (wait for the thread to finish)
    }

        this->activeObjectThread.reset(); // release unique ptr - before destruction join
    std::cout << "\nStage " << this->stageID << " (Active-Object):  Release smart pointer - Thread" << std::endl;
    std::cout << "\n********* FINISH Active Object " << stageID << " Stop Process *********" << std::endl;
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

// Set the task of the active object
void ActiveObject::setTaskHandler(std::function<void(std::weak_ptr<Graph>)> taskFunction)
{
    bool isValidTaskFunction = static_cast<bool>(taskFunction);
    if (isValidTaskFunction)
    {
        std::cout << "Set Task Handler for stage: " << stageID << std::endl;
        this->taskHandler = std::move(taskFunction); // Set the task handler to the provided handler
    }
    else
    {
        std::cerr << "Set Task Handler Failed, Stage: " << stageID << std::endl;
    }
}

// insert task to the queue
void ActiveObject::enqueueTask(std::weak_ptr<Graph> wptr_graph)
{
    if (wptr_graph.lock() != nullptr)
    {
        std::lock_guard<std::mutex> lock(this->mtx_AO); // Lock the mutex for the task queue
        this->queue_taskData.push(std::move(wptr_graph));
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
            stopProcess();
            return;
        }
        else
        {
            try{
                std::weak_ptr<Graph> wptr_graph;
                {
                    std::unique_lock<std::mutex> lock(this->mtx_AO); // Lock the mutex for the active task
                    wptr_graph = this->queue_taskData.front();
                    this->queue_taskData.pop();
                }
                this->taskHandler(wptr_graph); // Call the task handler

                // get the next stage shared ptr
                if (auto nextStagePtr = this->nextStage.lock())
                {
                    nextStagePtr->enqueueTask(wptr_graph);
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << "Error - Execute task: " << e.what() << std::endl;
            }
        }
    }
}

void ActiveObject::stopActiveObject()
{
    std::cout << "\n********* START Active Object " << this->stageID << " Stop Process *********" << std::endl;
    this->stop = true;
    if (!this->working)
    {
        this->cv_AO.notify_all();
    }
}

void ActiveObject::stopProcess()
{
    {
        std::unique_lock<std::mutex> lock(mtx_AO); // Lock the mutex
        
        while (!this->queue_taskData.empty())
        {
            this->queue_taskData.front().reset(); // release weak ptr
            this->queue_taskData.pop();
        }
        std::cout << "\nStage " << this->stageID << " (Active-Object):  Clean tasks queue" << std::endl;

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