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
    std::vector<std::unique_ptr<ActiveObjectDP>> stages;
    std::mutex pipeMtx;
    bool isStageActive;

    void setupPipe();
    void validateTaskExecution(int &client_FD, int choice);

public:
    PipeDP();
    ~PipeDP();
    void handleRequest(int& client_FD) override;
    void setStageActiveStatus(bool status);
};

#endif