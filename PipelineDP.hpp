#include <memory>
#include <vector>
#include <mutex>
#include "ActiveObjectDP.hpp"
#include "Graph.hpp"
#include "RequestService.hpp" 

class PipelineDP : public RequestService {
public:
    PipelineDP();
    ~PipelineDP();

    void handleRequest(int client_FD) override;

private:
    std::vector<std::unique_ptr<ActiveObjectDP>> stages;
    std::shared_ptr<Graph> graph;
    std::mutex graphMutex;

    std::vector<std::mutex> stageMutexes;
    std::vector<std::condition_variable> stageCVs;
    bool conditionMet[4] = {true, false, false, false};  //Stage 1 is always true, rest are false

    void setCondition(int stage);
    void waitForCondition(int stage);
};