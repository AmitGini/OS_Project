#include "PipelineDP.hpp"

#define NUM_STAGES 4

PipelineDP::PipelineDP() : stageCVs(NUM_STAGES), prevStageMutexes(NUM_STAGES) {
    // Create 4 stages
    // 1. Create graph
    // 2. Add/Remove edge
    // 3. Compute MST
    // 4. Get MST data
    this->graph = nullptr;
    for (int i = 0; i < NUM_STAGES; ++i) {
        stages.push_back(std::make_unique<ActiveObjectDP>());
    }
}

PipelineDP::~PipelineDP() {
    for (int i = 0; i < NUM_STAGES; ++i) {
        stages[i].reset();
    }
    stages.clear();
    stageCVs.clear();
    prevStageMutexes.clear();
    graphMutex.unlock();
    client.unlock();
    if (graph) {
        delete graph;
    }
}

void PipelineDP::handleRequest(int client_FD) {
    while (true) {
        std::lock_guard<std::mutex> lockFD(client);
        int choice = startConversation(client_FD);
        std::cout << "Choice: " << choice << std::endl;

        switch (choice) {
            case 1: // Create graph
                stages[0]->addTask([this, client_FD]() -> bool {
                    if(stages[0]->getMetConditionForNextStage()){
                        std::unique_lock<std::mutex> stageLock(prevStageMutexes[0]);
                        std::cout<<"Waiting for Stage 1 to complete"<<std::endl;
                        stageCVs[3].wait(stageLock, [this] { return stages[3]->waitForCompletion(); });
                    }
                    {
                        // std::lock_guard<std::mutex> lock(graphMutex);
                        if(createGraph(client_FD)) {
                            std::cout << "Task Stage 1 Done!" << std::endl;
                            stages[0]->setMetConditionForNextStage(true);  // Set condition for stage 1
                            stages[1]->setMetConditionForNextStage(false);  // Set condition for stage 2
                            stages[2]->setMetConditionForNextStage(false);  // Set condition for stage 3 has not met requirements 
                            stages[3]->setMetConditionForNextStage(false);
                            startNotifications(0);
                            return true;
                        }
                    }
                    return false;
                });
                break;

            case 2: // Add edge
            case 3: // Remove edge
                stages[1]->addTask([this, client_FD, choice]() -> bool {
                    if(!stages[0]->getMetConditionForNextStage()){
                        std::unique_lock<std::mutex> stageLock(prevStageMutexes[1]);
                        std::cout<<"Waiting for Stage 1 to complete"<<std::endl;
                        stageCVs[0].wait(stageLock, [this] { return stages[0]->waitForCompletion(); });
                    }
                    {
                        std::lock_guard<std::mutex> lock(graphMutex);
                        if(modifyGraph(client_FD, (choice == 2))){
                            std::cout<<"Task Stage 2 Done!"<<std::endl;
                            stages[1]->setMetConditionForNextStage(true);  // Set condition for stage 2
                            stages[2]->setMetConditionForNextStage(false);  // Set condition for stage 3 has not met requirements
                            stages[3]->setMetConditionForNextStage(false);  // Set condition for stage 4 has not met requirements
                        }
                        startNotifications(1);
                        return true;
                    }
                    startNotifications(1);
                    return false;
                });
                break;

            case 4: // Compute MST
                stages[2]->addTask([this, client_FD]() -> bool {
                    if(!stages[1]->getMetConditionForNextStage()){
                        std::unique_lock<std::mutex> stageLock(prevStageMutexes[2]);
                        std::cout<<"Waiting for Stage 2 to complete"<<std::endl;
                        stageCVs[1].wait(stageLock, [this] { return stages[1]->waitForCompletion(); });
                    }

                    std::lock_guard<std::mutex> lock(graphMutex);
                    if(calculateMST(client_FD)){
                        std::cout<<"Task Stage 3 Done!"<<std::endl;
                        stages[2]->setMetConditionForNextStage(true);
                        startNotifications(2);
                        return true;
                    }
                    startNotifications(2);
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
                        std::unique_lock<std::mutex> stageLock(prevStageMutexes[3]);
                        std::cout<<"Waiting for Stage 3 to complete"<<std::endl;
                        stageCVs[2].wait(stageLock, [this] { return stages[2]->waitForCompletion(); });
                    }

                    std::lock_guard<std::mutex> lock(graphMutex);
                    getMSTData(client_FD, choice);
                    std::cout<<"Task Stage 4 Done!"<<std::endl;
                    stages[3]->setMetConditionForNextStage(true);
                    startNotifications(3);
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

void PipelineDP::startNotifications(int currStage){
    std::unique_lock<std::mutex> stageLock;

    for(int i = 3; i > 0; --i){
        int prevStage = (currStage + i) % NUM_STAGES;
        stageLock = std::unique_lock<std::mutex>(prevStageMutexes[currStage]);

        if (stageCVs[prevStage].wait_for(stageLock, std::chrono::seconds(0)) != std::cv_status::timeout) 
            stageCVs[prevStage].notify_one();
        else std::cout<<"No one to notify for Stage "<<prevStage<<" To start Task in Stage"<< ((prevStage+1)%NUM_STAGES)<<std::endl;
        break;
    }
 }
