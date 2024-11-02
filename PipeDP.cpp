#include "PipeDP.hpp"

PipeDP::PipeDP() : stageWorking(false) {
    setupPipe();
}

PipeDP::~PipeDP() {
    std::cout<<"Deleting PipeDP"<<std::endl;
    for(auto& stage : stages) {
        // Reset the unique pointer, so that the memory is released and the destructor of the object is called.
        stage.reset();
    }
    
    if (this->graph) {
        delete this->graph;
    }
}

void PipeDP::setupPipe() {
    // Create 4 stages
    for(int i = 0; i < 5; i++) {
        stages.push_back(std::make_unique<ActiveObjectDP>());
    }
    
    // Set up the pipeline connections
    stages[0]->setNextStage(stages[1].get());  // stage to handle graph creation
    stages[1]->setNextStage(stages[2].get());  // stage to handle graph modification
    stages[2]->setNextStage(stages[3].get());  // stage to handle MST calculation
    stages[3]->setNextStage(stages[3].get());  // stage to handle MST operations
    stages[4]->setNextStage(stages[4].get());  // stage to handle client exit
    
    stages[0]->setPrevStageStatus(true);
    stages[4]->setPrevStageStatus(true);

    // Define task handlers for each stage
    stages[0]->setTaskHandler([this](int client_fd, int choice) -> bool {
        stages[1]->updateNextStage(false);
        stages[2]->updateNextStage(false);
        return createGraph(client_fd);
    });

    stages[1]->setTaskHandler([this](int client_fd, int choice) -> bool {
        bool oneOfTwo = choice == 2;
        stages[2]->updateNextStage(false);
        stages[3]->updateNextStage(false);
        return modifyGraph(client_fd, oneOfTwo);
    });

    stages[2]->setTaskHandler([this](int client_fd, int choice) -> bool {
        return calculateMST(client_fd);
    });

    stages[3]->setTaskHandler([this](int client_fd, int choice) -> bool {
        getMSTData(client_fd, choice);
        return true;
    });

    stages[4]->setTaskHandler([this](int client_fd, int choice) -> bool {
        for(int i = 3; i >= 0; i--){
            stages[i]->enqueueClientTasks(client_fd);
        }
        return stopClient(client_fd);
    });
}

void PipeDP::handleRequest(int client_FD) {
    while (true) {
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
                stages[4]->enqueue(client_FD, choice);
                return;
        }
        
        validateTaskExecution();
    }
}

void PipeDP::validateTaskExecution() {
    
    for(int i = 4; i >= 0; i--) {
        std::cout<<"************* Stage "<<i<<" *************"<<std::endl;
        std::cout<<"Trying to Notify Stage"<<std::endl;
        stages[i]->notify();
        if(i < 3)stages[i]->makePipeWait(this->pipeMtx, this);

        std::cout<<"************************************"<<std::endl;
    }
}

void PipeDP::setStageWorkStatus(bool status) {
     std::cout<<"Pipe Updated Working Status From: "<<this->stageWorking<<" To: "<<status<<std::endl; 
    this->stageWorking = status;
}
