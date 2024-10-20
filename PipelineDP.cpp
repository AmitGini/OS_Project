#include "PipelineDP.hpp"

PipelineDP::PipelineDP() {
    setupPipeline();
}

PipelineDP::~PipelineDP() {
    if (this->graph) {
        delete this->graph;
    }
}

void PipelineDP::setupPipeline() {
    // Create 4 stages
    for(int i = 0; i < 4; i++) {
        stages.push_back(std::make_unique<ActiveObjectDP>());
    }
    
    // Set up the pipeline connections
    stages[0]->setNextStage(stages[1].get());
    stages[1]->setNextStage(stages[2].get());
    stages[2]->setNextStage(stages[3].get());
    stages[3]->setNextStage(stages[3].get());
    stages[0]->setPrevStageStatus(true);

    // Define task handlers for each stage
    stages[0]->setTaskHandler([this](int client_fd, int choice) -> bool {
        std::lock_guard<std::mutex> lock(graphMutex);
        stages[1]->updateNextStage(false);
        stages[2]->updateNextStage(false);
        return createGraph(client_fd);
    });

    stages[1]->setTaskHandler([this](int client_fd, int choice) -> bool {
        bool oneOfTwo = choice == 2;
        std::lock_guard<std::mutex> lock(graphMutex);
        stages[2]->updateNextStage(false);
        stages[3]->updateNextStage(false);
        return modifyGraph(client_fd, oneOfTwo);
    });

    stages[2]->setTaskHandler([this](int client_fd, int choice) -> bool {
        std::lock_guard<std::mutex> lock(graphMutex);
        return calculateMST(client_fd);
    });

    stages[3]->setTaskHandler([this](int client_fd, int choice) -> bool {
        std::lock_guard<std::mutex> lock(graphMutex);
        getMSTData(client_fd, choice);
        return true;
    });
}

void PipelineDP::handleRequest(int client_FD) {
    while (true) {
        std::lock_guard<std::mutex> lockClient(clientMutex);
        int choice = startConversation(client_FD);
        
        switch (choice) {
            case 1: // Create graph
                stages[0]->enqueue(client_FD, choice);
                break;
            case 2: // Add edge
            case 3: // Remove edge
                stages[1]->enqueue(client_FD, choice);
                break;
            case 4: // Compute MST
                stages[2]->enqueue(client_FD, choice);
                break;
            case 5: // MST operations
            case 6:
            case 7:
            case 8:
            case 9:
                stages[3]->enqueue(client_FD, choice);
                break;
            case 10: // Exit
                close(client_FD);
                return;
        }
        validateTaskExecution();
    }
}

void PipelineDP::validateTaskExecution() {
    for(int i = 3; i >= 0; i--) {
        stages[i]->notify();
    }
}