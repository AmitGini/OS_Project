#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "ActiveObject.hpp"

class Pipeline
{
private:
    // Private members
    std::vector<std::shared_ptr<ActiveObject>> stages; // Vector of active objects (we used shared_ptr to avoid memory leaks)
    std::mutex mtx;                                // Mutex for the pipe

    // Private methods
    void createAOStages(); // Setup the pipe with active objects
    void setAONextStage(); // Set the next stage for each active object
    void setTaskHandler(); // Set the task handler for each active object

public:
    Pipeline();
    ~Pipeline();

    void processGraphs(std::vector<std::weak_ptr<Graph>>& graphs); // Process the graphs that sended from the server
};

#endif