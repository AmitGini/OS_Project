#ifndef PIPEDP_HPP
#define PIPEDP_HPP

#include <vector> 
#include <memory> // For shared_ptr
#include <mutex>
#include <condition_variable> // For condition_variable
#include "RequestService.hpp"
#include "ActiveObjectDP.hpp"
#include "TaskQueue.hpp"

class PipeDP : public RequestService {
private:
    // Private members
    std::vector<std::shared_ptr<ActiveObjectDP>> stages; // Vector of active objects (we used shared_ptr to avoid memory leaks)
    std::mutex pipeMtx; // Mutex for the pipe
    std::atomic<bool> isStageActive; // Flag to check if the stage is active (we used atomic for thread safety)

    // Private methods
    void setupPipe();  // Setup the pipe with active objects
public:
    PipeDP();
    ~PipeDP();
    void handleRequest(int& client_FD) override;
    void setStageActiveStatus(bool status);
};

#endif