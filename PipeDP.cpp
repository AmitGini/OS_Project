#include "PipeDP.hpp"

#define STAGE_CREATE_GRAPH 0
#define STAGE_MODIFY_GRAPH 1
#define STAGE_CALCULATE_MST 2
#define STAGE_MST_OPERATIONS 3
#define STAGE_CLIENT_EXIT 4
#define STAGE_UNDEFINE -1 // Undefined stage for error handling

PipeDP::PipeDP() : isStageActive(false) {
    setupPipe();
}

PipeDP::~PipeDP() {
    std::cout<<"**** Closing Pattern Type ****"<<std::endl;
    std::cout<<"Deleting PipeDP"<<std::endl;
    this->isStageActive = false;
    stages.clear();
    std::cout<<"Pipe And Active Objects has Closed"<<std::endl;
}

// define the task handlers for each stage
void PipeDP::setupPipe() {
    try {
        // Pre-allocate vector capacity
        stages.reserve(STAGE_CLIENT_EXIT + 1); 
        
        // Create stages with error checking
        for(int i = STAGE_CREATE_GRAPH; i < STAGE_CLIENT_EXIT+ 1; ++i) {
            std::cout <<"***** "<< "Creating stage " << i <<" *****"<<std::endl;
            stages.push_back(std::make_shared<ActiveObjectDP>(i)); // Create a shared pointer to an active object
            std::cout << "Stage " << i << " created successfully" << std::endl;
        }
        
        std::cout<<"*******Here*******"<<std::endl;

        // Set up the pipeline connections
        stages[STAGE_CREATE_GRAPH]->setNextStage(stages[STAGE_MODIFY_GRAPH]);
        stages[STAGE_MODIFY_GRAPH]->setNextStage(stages[STAGE_CALCULATE_MST]);
        stages[STAGE_CALCULATE_MST]->setNextStage(stages[STAGE_MST_OPERATIONS]);
        stages[STAGE_MST_OPERATIONS]->setNextStage(stages[STAGE_CLIENT_EXIT]);
        stages[STAGE_CLIENT_EXIT]->setNextStage(stages[STAGE_CLIENT_EXIT]);

        stages[STAGE_CREATE_GRAPH]->setPrevStageStatus(true); // Set the first stage to active (because it's the first stage)

        // Define task handlers for each stage
        stages[STAGE_CREATE_GRAPH]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            std::cout<<"Set Task Handler for stage 0"<<std::endl;
            return createGraph(client_FD);
            
            
        });

        stages[STAGE_MODIFY_GRAPH]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            bool addOrRemoveEdge = (choice == 2); // 2 for add edge, 3 for remove edge
            std::cout<<"Set Task Handler for stage 1"<<std::endl;
            return modifyGraph(client_FD, addOrRemoveEdge);

        });

        stages[STAGE_CALCULATE_MST]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            std::cout<<"Set Task Handler for stage 2"<<std::endl;
            return calculateMST(client_FD);
            

        });

        stages[STAGE_MST_OPERATIONS]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            std::cout<<"Set Task Handler for stage 3"<<std::endl;
            getMSTData(client_FD, choice);
            return true; 
        });

        stages[STAGE_CLIENT_EXIT]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            std::cout<<"Set Task Handler for stage 4"<<std::endl;
            return stopClient(client_FD);
            
        });
        
    } catch (const std::exception& e) {
        std::cerr << "Stage creation failed: " << e.what() << std::endl;
        stages.clear();
        throw;
    }
}

void PipeDP::handleRequest(int& client_FD) {
    while (client_FD > 0) {
        int choice = startConversation(client_FD);
        
        int numStage = (choice == 1) ? STAGE_CREATE_GRAPH : 
            (choice == 2 || choice == 3) ? STAGE_MODIFY_GRAPH :
            (choice == 4) ? STAGE_CALCULATE_MST :
            (choice >= 5 && choice <= 9) ? STAGE_MST_OPERATIONS :
            (choice == 10) ? STAGE_CLIENT_EXIT : STAGE_UNDEFINE;
        
        std::cout<<"\n************* Stage "<<numStage<<" *************"<<std::endl;

        switch (choice) {
            case 1: // Create graph
                this->stages[STAGE_CREATE_GRAPH]->enqueue(client_FD, choice);
                break;
            case 2: // Add edge
            case 3: // Remove edge
                this->stages[STAGE_MODIFY_GRAPH]->enqueue(client_FD, choice);
                break;
            case 4: // Compute MST
                this->stages[STAGE_CALCULATE_MST]->enqueue(client_FD, choice);
                break;
            case 5: // MST operations
            case 6:
            case 7:
            case 8:
            case 9:
                this->stages[STAGE_MST_OPERATIONS]->enqueue(client_FD, choice);
                break;
            case 10: // Exit
                this->stages[STAGE_CLIENT_EXIT]->enqueue(client_FD, choice);
                return;
        }
        
        // Wait for the stages to finish their work
        for(auto& stage : stages) {
            if(stage->isActive()) {
                stage->makePipeWait(this->pipeMtx, this);
                sleep(1);  // Sleep for 1 second in case next stages are still active, to avoid conition race
            }
        }
        
        std::cout<<"************************************"<<std::endl;
    }
}

void PipeDP::setStageActiveStatus(bool status) {
     std::cout<<"Pipe Updated Working Status From: "<<this->isStageActive<<" To: "<<status<<std::endl; 
    this->isStageActive = status;
}
