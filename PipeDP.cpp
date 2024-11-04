#include "PipeDP.hpp"

#define STAGE_CREATE_GRAPH 0
#define STAGE_MODIFY_GRAPH 1
#define STAGE_CALCULATE_MST 2
#define STAGE_MST_OPERATIONS 3
#define STAGE_CLIENT_EXIT 4
#define STAGE_UNDEFINE -1

PipeDP::PipeDP() : isStageActive(false) {
    setupPipe();
}

PipeDP::~PipeDP() {
    std::cout<<"**** Deleting Pipe ****"<<std::endl;
    {
        std::lock_guard<std::mutex> lock(this->pipeMtx);
        this->isStageActive = false;
        std::cout<<"Pipe: Make Sure All Stages Done Working"<<std::endl;
        for(auto& stage : stages) {
            if(stage->isActive()) {
                stage->makePipeWait(this->pipeMtx, this);
                sleep(0.5);  // Sleep for 1 second in case next stages are still active, to avoid conition race
            }
            stage->clearActiveObject();  // Clear queue and reset the shared pointer of the queue and next stage
        }
    }
    if(!isStageActive){
        std::cout<<"Pipe: All Stages Done Working"<<std::endl;
        for(auto stage : stages) {
            if(stage){
                stage.reset();  // Reset the shared pointer
            }
        }
        std::cout<<"Pipe: Clear Stages"<<std::endl;
        stages.clear();  // Clear the vector of stages
    } else {
        std::cout<<"********* Failed to Close PipeDP *********"<<std::endl;
    }
    
    if(graph){
        std::cout<<"Pipe: Close Graph"<<std::endl;
        this->graph->clearGraph();  // Clear the graph
        this->graph.reset();  // Reset the shared pointer of the graph
    }
    std::cout<<"********* Pipe: Done *********"<<std::endl;
}

void PipeDP::setupPipe() {
    try {

        stages.reserve(STAGE_CLIENT_EXIT + 1);
        
        for(int i = STAGE_CREATE_GRAPH; i <= STAGE_CLIENT_EXIT; ++i) {
            std::cout <<"***** "<< "Creating stage " << i <<" *****"<<std::endl;
            auto stage = std::make_shared<ActiveObjectDP>();
            if (!stage) {
                throw std::runtime_error("Failed to create stage");
            }
            stages.push_back(std::move(stage));  // Use move semantics
            std::cout << "Stage " << i << " created successfully" << std::endl;
        }
        
        // Set up the pipeline connections
        stages[STAGE_CREATE_GRAPH]->setNextStage(stages[STAGE_MODIFY_GRAPH]);
        stages[STAGE_MODIFY_GRAPH]->setNextStage(stages[STAGE_CALCULATE_MST]);
        stages[STAGE_CALCULATE_MST]->setNextStage(stages[STAGE_MST_OPERATIONS]);
        stages[STAGE_MST_OPERATIONS]->setNextStage(stages[STAGE_CLIENT_EXIT]);
        stages[STAGE_CLIENT_EXIT]->setNextStage(stages[STAGE_CLIENT_EXIT]);

        std::cout<<"Finish Set Next Stages"<<std::endl;

        stages[STAGE_CREATE_GRAPH]->setPrevStageStatus(true);

        // Define task handlers for each stage
        stages[STAGE_CREATE_GRAPH]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            stages[STAGE_MODIFY_GRAPH]->updateNextStage(false);
            stages[STAGE_CALCULATE_MST]->updateNextStage(false);
            return createGraph(client_FD);
        });

        stages[STAGE_MODIFY_GRAPH]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            bool addOrRemoveEdge = (choice == 2); // 2 for add edge, 3 for remove edge
            stages[STAGE_CALCULATE_MST]->updateNextStage(false);
            stages[STAGE_MST_OPERATIONS]->updateNextStage(false);
            return modifyGraph(client_FD, addOrRemoveEdge);
        });

        stages[STAGE_CALCULATE_MST]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            return calculateMST(client_FD);
        });

        stages[STAGE_MST_OPERATIONS]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            getMSTData(client_FD, choice);
            return true;
        });

        stages[STAGE_CLIENT_EXIT]->setTaskHandler([this](int &client_FD, int choice) -> bool {
            return stopClient(client_FD);
        });
        std::cout<<"Set Task Handler"<<std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Stage creation failed: " << e.what() << std::endl;
        for(auto& stage : stages) {
            stage.reset();
        }
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
                this->stages[STAGE_CREATE_GRAPH]->notify();
                break;
            case 2: // Add edge
            case 3: // Remove edge
                this->stages[STAGE_MODIFY_GRAPH]->enqueue(client_FD, choice);
                this->stages[STAGE_MODIFY_GRAPH]->notify();
                break;
            case 4: // Compute MST
                this->stages[STAGE_CALCULATE_MST]->enqueue(client_FD, choice);
                this->stages[STAGE_CALCULATE_MST]->notify();
                break;
            case 5: // MST operations
            case 6:
            case 7:
            case 8:
            case 9:
                this->stages[STAGE_MST_OPERATIONS]->enqueue(client_FD, choice);
                this->stages[STAGE_MST_OPERATIONS]->notify();
                break;
            case 10: // Exit
                this->stages[STAGE_CLIENT_EXIT]->enqueue(client_FD, choice);
                this->stages[STAGE_CLIENT_EXIT]->notify();
                return;
        }
        
        for(auto& stage : stages) {
            if(stage->isActive()) {
                stage->makePipeWait(this->pipeMtx, this);
                sleep(0.5);  // Sleep for 1 second in case next stages are still active, to avoid conition race
            }
        }
        
        std::cout<<"************************************"<<std::endl;
    }
}

void PipeDP::setStageActiveStatus(bool status) {
     std::cout<<"Pipe Updated Working Status From: "<<this->isStageActive<<" To: "<<status<<std::endl; 
    this->isStageActive = status;
}
