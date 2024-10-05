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
    void startNotifications(int stage);
private:
    std::vector<std::unique_ptr<ActiveObjectDP>> stages;
    std::vector<std::mutex> prevStageMutexes;
    std::vector<std::condition_variable> stageCVs;
    std::mutex graphMutex;
    std::mutex client;

};