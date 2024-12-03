#include "LeaderFollower.hpp"

#define NO_PROCESS -1
#define AT_PROCESS 0
#define FINISH_PROCESS 1

// Constructor
LeaderFollower::LeaderFollower() : stop(false)
{

    std::lock_guard<std::mutex> lock(this->mtx_lf);
    std::cout << "Starting Leader Follower Design Pattern" << std::endl;
    for (int i = 0; i < this->numThreads.load(); i++)
    {
        this->threadsPool.push(std::make_unique<std::thread>(&LeaderFollower::work, this));
    }
    std::cout << "Leader-Follower: Threads Created and Added to Pool." << std::endl;
}


// Destructor
LeaderFollower::~LeaderFollower()
{
    {
        std::lock_guard<std::mutex> lock(this->mtx_lf);
        std::cout << "\n********* START Leader-Follower Stop Process *********" << std::endl;
        this->stop = true;
    }

    while (!this->queue_taskData.empty())
    {
        std::lock_guard<std::mutex> lock(this->mtx_lf);
        this->queue_taskData.pop();
    }

    {
        std::lock_guard<std::mutex> lock(this->mtx_lf);
        std::cout << "\nLeader-Follower: Task Queue is Empty" << std::endl;
    }
    this->cv_lf.notify_all(); // Notify all threads to exit

    while (!this->threadsPool.empty())
    {
        auto &thread = this->threadsPool.front();
        if (thread->joinable())
        {
            thread->join();
        }
        this->threadsPool.pop();
    }
    std::lock_guard<std::mutex> lock(this->mtx_lf);
    std::cout << "\nLeader-Follower: Threads Pool is Clean and Threads Joined" << std::endl;
    std::cout << "\n********* FINISH Leader-Follower Stop Process *********" << std::endl;
}

// Function that Recive data from the server to process
void LeaderFollower::processGraphs(std::vector<std::weak_ptr<Graph>>& graphs)
{
    {
        std::lock_guard<std::mutex> lock(this->mtx_lf);
        for (const auto& graph : graphs)
        {
            this->queue_taskData.push(graph);
        }
        std::cout << "Leader-Follower: Graphs Added to Task Queue." << std::endl;
    }
    promoteFollower();
}

// Start the conversation with the client
void LeaderFollower::work() 
{
    while (!this->stop) 
    {
        {
            std::unique_lock<std::mutex> lock(this->mtx_lf);
            this->cv_lf.wait(lock, [this]
                { return  (!this->queue_taskData.empty() && isLeader())  || this->stop; });
        }
        if (this->stop) 
        {
            return;
        } 
        else 
        {
            executeTask();
            promoteFollower();
        }
    }
}

void LeaderFollower::executeTask() 
{
    std::shared_ptr<Graph> currentGraph;
    {
        std::lock_guard<std::mutex> lock(this->mtx_lf);
        if (this->queue_taskData.empty()) return;
        /* 
            If the graph is valid  (return value of lock is not NULL) 
                Do:
                Set currenGraph, pop and continue to execute 
                Else:
                just pop from the queue and return
        */
        if (auto graph = this->queue_taskData.front().lock()) 
        {
            currentGraph = graph;
            this->queue_taskData.pop();
        } 
        else 
        {
            this->queue_taskData.pop();
            return;
        }
    }

    // Process the graph
    currentGraph->setMSTDataCalculationNextStatus();
    currentGraph->setMSTTotalWeight();
    currentGraph->setMSTLongestDistance();
    currentGraph->setMSTShortestDistance();
    currentGraph->setMSTAvgEdgeWeight();
    currentGraph->setMSTDataCalculationNextStatus();
}

void LeaderFollower::promoteFollower() 
{
    std::unique_lock<std::mutex> lock(this->mtx_lf);
    // If the current leader is not empty, then push the current leader (in front) to the back of the queue
    if(isLeader()) 
    {
        this->threadsPool.push(std::move(this->threadsPool.front()));
        this->threadsPool.pop();
    }

    lock.unlock();
    // If the queue is empty, then set the leader to empty
    if (this->queue_taskData.empty()) 
    {
        setLeader(std::thread::id());
    }
    else  // Else, set the leader to the front of the queue
    {
        setLeader(this->threadsPool.front()->get_id());
        this->cv_lf.notify_all();
    }
}

bool LeaderFollower::isLeader() 
{
    return std::this_thread::get_id() == currentLeader.load();
}

void LeaderFollower::setLeader(std::thread::id id) 
{
    std::lock_guard<std::mutex> lock(this->mtx_lf);
    currentLeader.store(id);
}