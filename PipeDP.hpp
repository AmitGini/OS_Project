#ifndef PIPEDP_HPP
#define PIPEDP_HPP

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "RequestService.hpp"
#include "ActiveObjectDP.hpp"
#include "FunctionQueue.hpp"

class PipeDP : public RequestService {
private:
    std::vector<std::unique_ptr<ActiveObjectDP>> stages;
    std::mutex pipeMtx;
    bool stageWorking;

    void setupPipeline();
    void validateTaskExecution();

public:
    PipeDP();
    ~PipeDP();
    void handleRequest(int client_FD) override;
    void setStageWorkStatus(bool status);
};

#endif