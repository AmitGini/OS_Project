#include "LeaderFollower.hpp"

#define NO_PROCESS -1
#define AT_PROCESS 0
#define FINISH_PROCESS 1

// Constructor
LeaderFollower::LeaderFollower() : stop(false)
{
    try
    {
        for (int i = 0; i < numThreads.load(); i++)
        {
            this->threadsPool.push(std::make_unique<std::thread>(&LeaderFollower::work, this));
        }
        std::cout << "Threads Created and Added to Pool." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


// Destructor
LeaderFollower::~LeaderFollower()
{
    std::cout << "\nStopping Leader-Follower framework..." << std::endl;
    this->stop = true;
    this->cv_lf.notify_all(); // Notify all threads to exit

    while (!this->queue_taskData.empty())
    {
        this->queue_taskData.pop();
    }

    while (!this->threadsPool.empty())
    {
        auto &thread = this->threadsPool.front();
        if (thread->joinable())
        {
            thread->join();
        }
        this->threadsPool.pop();
    }
    std::cout << "\nLeader-Follower: Threads Pool is Clean and Threads Detached" << std::endl;
    std::cout << "\n********* FINISH Leader-Follower Stop Process *********" << std::endl;
}

// Function that Recive data from the server to process
void LeaderFollower::processGraphs(std::vector<std::weak_ptr<Graph>>& graphs)
{
    {
        std::lock_guard<std::mutex> lock(this->mtx_lf);
        for (const auto& graph : this->graphs)
        {
            this->queue_taskData.push(graph);
        }
    }
    promoteFollower();
}

// Start the conversation with the client
void LeaderFollower::work() {
    while (!this->stop) {
        std::unique_lock<std::mutex> lock(this->mtx_lf);
        this->cv_lf.wait(lock, [this]
            { return  (!this->queue_taskData.empty() && isLeader())  || this->stop; });

        if (this->stop) 
        {
            return;
        } else {
            executeTask();
            promoteFollower();
        }
    }
}

void LeaderFollower::executeTask() {
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
        if (auto graph = this->queue_taskData.front().lock()) {
            currentGraph = graph;
            this->queue_taskData.pop();
        } else {
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

void LeaderFollower::promoteFollower() {
    std::lock_guard<std::mutex> lock(this->mtx_lf);
    
    if (!this->queue_taskData.empty()) {
        // Keep track of current thread index in an atomic variable
        static std::atomic<int> currentThreadIndex{0};
        int nextIndex = (currentThreadIndex.fetch_add(1) + 1) % this->numThreads;
        setLeader(this->threadsPool[nextIndex]->get_id());
        this->cv_lf.notify_all();
    } else {
        setLeader(std::thread::id());
    }
}

bool LeaderFollower::isLeader() {
    return std::this_thread::get_id() == currentLeader.load();
}

void LeaderFollower::setLeader(std::thread::id id) {
    currentLeader.store(id);
}