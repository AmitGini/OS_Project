#ifndef PIPELINEDP_HPP
#define PIPELINEDP_HPP

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "RequestService.hpp"
#include "ActiveObjectDP.hpp"
#include "FunctionQueue.hpp"

class PipelineDP : public RequestService {
private:
    std::vector<std::unique_ptr<ActiveObjectDP>> stages;
    std::mutex pipeMtx;
    bool stageWorking;

    void setupPipeline();
    void validateTaskExecution();

public:
    PipelineDP();
    ~PipelineDP();
    void handleRequest(int client_FD) override;
    void setStageWorkStatus(bool status);
};

#endif