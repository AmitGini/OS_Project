#ifndef PIPEDP_HPP
#define PIPEDP_HPP

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "RequestService.hpp"
#include "ActiveObjectDP.hpp"
#include "TaskQueue.hpp"

class PipeDP : public RequestService {
private:
    // Private members
    std::vector<std::shared_ptr<ActiveObjectDP>> stages;
    std::mutex pipeMtx;
    std::atomic<bool> isStageActive;

    // Private methods
    void setupPipe();  // Setup the pipe with active objects
    
public:
    PipeDP();
    ~PipeDP();
    void handleRequest(int& client_FD) override;
    void setStageActiveStatus(bool status);
};

#endif