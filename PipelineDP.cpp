#include "PipelineDP.hpp"

#define NUM_STAGES 4

PipelineDP::PipelineDP() : graph(nullptr), stageCVs(NUM_STAGES), stageMutexes(NUM_STAGES) {
    // Create 4 stages
    // 1. Create graph
    // 2. Add/Remove edge
    // 3. Compute MST
    // 4. Get MST data
    for (int i = 0; i < NUM_STAGES; ++i) {
        stages.push_back(std::make_unique<ActiveObjectDP>());
    }
}

PipelineDP::~PipelineDP() = default;

void PipelineDP::handleRequest(int client_FD) {
    while (true) {
        int choice = startConversation(client_FD);
        std::cout << "Choice: " << choice << std::endl;

        switch (choice) {
            case 1: // Create graph
                stages[0]->addTask([this, client_FD]() -> bool {
                    std::lock_guard<std::mutex> lock(graphMutex);
                    if(createGraph(client_FD)) {
                        std::cout << "Task Stage 1 Done!" << std::endl;
                        stages[0]->setMetConditionForNextStage(true);  // Set condition for stage 1
                        stages[1]->setMetConditionForNextStage(false);  // Set condition for stage 2
                        stages[2]->setMetConditionForNextStage(false);  // Set condition for stage 3 has not met requirements 
                        stages[3]->setMetConditionForNextStage(false);
                        stageCVs[0].notify_one();
                        return true;
                    }
                    return false;
                });
                break;
            case 2: // Add edge
            case 3: // Remove edge
                stages[1]->addTask([this, client_FD, choice]() -> bool {
                    if(!stages[0]->getMetConditionForNextStage()){
                        std::unique_lock<std::mutex> stageLock(stageMutexes[0]);
                        std::cout<<"Waiting for Stage 1 to complete"<<std::endl;
                        stageCVs[0].wait(stageLock, [this] { return stages[0]->waitForCompletion(); });
                    }
                    std::lock_guard<std::mutex> lock(graphMutex);
                    if(modifyGraph(client_FD, (choice == 2))){
                        std::cout<<"Task Stage 2 Done!"<<std::endl;
                        stages[1]->setMetConditionForNextStage(true);  // Set condition for stage 2
                        stages[2]->setMetConditionForNextStage(false);  // Set condition for stage 3 has not met requirements
                        stages[3]->setMetConditionForNextStage(false);  // Set condition for stage 4 has not met requirements
                        stageCVs[1].notify_one();
                        return true;
                    }
                    return false;
                });
                break;
            case 4: // Compute MST
                stages[2]->addTask([this, client_FD]() -> bool {
                    if(!stages[1]->getMetConditionForNextStage()){
                        std::unique_lock<std::mutex> stageLock(stageMutexes[1]);
                        std::cout<<"Waiting for Stage 2 to complete"<<std::endl;
                        stageCVs[1].wait(stageLock, [this] { return stages[1]->waitForCompletion(); });
                    }
                    
                    std::lock_guard<std::mutex> lock(graphMutex);
                    if(calculateMST(client_FD)){
                        std::cout<<"Task Stage 3 Done!"<<std::endl;
                        stages[2]->setMetConditionForNextStage(true);
                        stageCVs[2].notify_one();
                        return true;
                    }
                    return false;
                });
                break;
            case 5: // Longest Weighted path in MST
            case 6: // Shortest Weighted path in MST
            case 7: // Average Weight of edges in MST
            case 8: // Total Weight of the MST
            case 9: // Print MST
                stages[3]->addTask([this, client_FD, choice]() -> bool {
                    if(!stages[2]->getMetConditionForNextStage())
                    {   
                        std::unique_lock<std::mutex> stageLock(stageMutexes[2]);
                        std::cout<<"Waiting for Stage 3 to complete"<<std::endl;
                        stageCVs[2].wait(stageLock, [this] { return stages[2]->waitForCompletion(); });
                    }
                    std::lock_guard<std::mutex> lock(graphMutex);
                    getMSTData(client_FD, choice);
                    std::cout<<"Task Stage 4 Done!"<<std::endl;
                    stages[3]->setMetConditionForNextStage(true);
                    stageCVs[3].notify_all();
                    return true;
                    
                });
                break;
            case 10: // Exit
                close(client_FD);
                return;
            default:
                break;
        }
    }
}

void PipelineDP::waitForCondition(int stage) {
    std::unique_lock<std::mutex> lock(stageMutexes[stage]);
    stageCVs[stage].wait(lock, [this, stage] { return conditionMet[stage]; });
}

void PipelineDP::setCondition(int stage) {
    std::lock_guard<std::mutex> lock(stageMutexes[stage]);
    conditionMet[stage] = true;
    stageCVs[stage].notify_one();
}