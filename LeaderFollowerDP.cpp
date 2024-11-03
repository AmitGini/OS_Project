#include "LeaderFollowerDP.hpp"
#define NO_LEADER -1
#define CONVERSATION 0
#define TASKS 1
/*
Working with 2 threads, one leader that handle the conversation with the client and preform the enqueue task.
And the other thread is the follower that will execute the task.
If the leader is done with the conversation, the follower will be promoted to leader.
If the leader is done execute all the tasks and queue is empty or cant preform the task, the follower will be promoted to leader.
*/

LeaderFollowerDP::LeaderFollowerDP() : leaderIndex(NO_LEADER), stop(false), client_fd(-1) {
    try{
        this->threadsPool.push_back(std::thread(&LeaderFollowerDP::tasksEnqueing, this));
        this->threadsPool.push_back(std::thread(&LeaderFollowerDP::tasksExecution, this));
        std::cout<<"Created Threades"<<std::endl;
    }catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

LeaderFollowerDP::~LeaderFollowerDP() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->stop = true;
        this->toCloseClient = true;
    }
    
    cv.notify_all();
    
    for (auto& thread : threadsPool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    tasksQueue.clear();
    std::cout<<"Leader Follower Threads has Closed!"<<std::endl;
}

void LeaderFollowerDP::handleRequest(int& client_FD) {
    this->toCloseClient = false;
    std::cout<<"\n*** Leader-Follower - New Connection ***"<<std::endl;
    this->client_fd = client_FD;
    promoteFollower(CONVERSATION);
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        this->cv.wait(lock, [this] { return (this->toCloseClient || this->stop); });
    }
}

void LeaderFollowerDP::tasksEnqueing(){
    while(true){
        if(this->leaderIndex != CONVERSATION){
            std::unique_lock<std::mutex> lock(this->mtx);
            std::cout<<"** Conversation Follower ** - Waiting For Promotion"<<std::endl;
            this->cv.wait(lock, [this] { return (this->leaderIndex == CONVERSATION || this->stop); });
        }

        if (stop) break;

        if(this->client_fd < 0){
            std::cout<<"\n** Client has Left **\n"<<std::endl;
            this->toCloseClient = true;
            promoteFollower(NO_LEADER);
            continue;
        }
        
        try{
            if(this->toCloseClient){
                promoteFollower(NO_LEADER);
            }else{
                std::cout<<"Starting Conversation with Client: "<<client_fd<<std::endl;
                int choice = this->startConversation(this->client_fd);
                //If return false - client choice is 0 to execute tasks or 10 to exit
                if(!enqueingChoices(choice)) {  
                    std::cout<<"\nPromoting Follower to Execute Tasks\n"<<std::endl;
                    promoteFollower(TASKS);
                }
            }
        }catch(const std::exception& e){
            int follower = (client_fd < 0) ? NO_LEADER : CONVERSATION;
            promoteFollower(follower);
        }
    }
}

void LeaderFollowerDP::tasksExecution(){
    while(true){
        if(this->leaderIndex != TASKS){
            std::unique_lock<std::mutex> lock(this->mtx);
            std::cout<<"** Task Follower ** - Waiting For Promotion"<<std::endl;
            this->cv.wait(lock, [this] { return this->leaderIndex == TASKS || this->stop; });
        }

        if (stop) break;

        if(this->client_fd < 0){
            std::cout<<"\n** Client has Left **\n"<<std::endl;
            this->toCloseClient = true;
            promoteFollower(NO_LEADER);
            continue;
        }

        try{
            if(this->toCloseClient){
                promoteFollower(NO_LEADER);
            }
            else if(this->tasksQueue.isEmpty()){
                std::cout<<"Tasks Queue is Empty"<<std::endl;
                promoteFollower(CONVERSATION);
            }else{
                bool success = this->tasksQueue.executeTask();
                if(!success){
                    sendMessage(this->client_fd, "Task execution failed. Try to use Healthy Logic. (Recommended)\n");
                    std::cout<<"*** Task Execution Failed - Dequeue and Keep Working ***"<<std::endl;
                }
            }
        }catch(const std::exception& e){
            int follower = (client_fd < 0) ? NO_LEADER : TASKS;
            promoteFollower(follower);
        }
    }
}

bool LeaderFollowerDP::enqueingChoices(int choice){
    TaskQueue::TaskType func;
    std::string message;

    switch (choice) {
        case 1: // Create graph
            //define task to add to the equeue
            func = [this](int &client_fd, int choice) -> bool {
                return createGraph(client_fd);};
            break;
        
        case 2: // Add edge
        case 3: // Remove edge
            func = [this](int &client_fd, int choice) -> bool {
                return modifyGraph(client_fd, choice == 2);};
            break;

        case 4: // Calculate MST
            func = [this](int &client_fd, int choice) -> bool {
                return calculateMST(client_fd);};
            break;

        case 5: // MST operations
        case 6:
        case 7:
        case 8:
        case 9:
            func = [this](int &client_fd, int choice) -> bool {
                return getMSTData(client_fd, choice);};
            break;
        
        case 10: // Client Exit 
            func = [this](int &client_fd, int choice) -> bool {
                this->toCloseClient = true;
                return this->stopClient(client_fd);};
            
            this->tasksQueue.clear();  // Clear the queue of tasks client might entered
            this->tasksQueue.enqueue(func , this->client_fd, choice);
            return false;  // return value false execute the tasks
            
        case 0: // Start Executing the tasks 
            return false; // return value false execute the tasks

        default:
            sendMessage(this->client_fd, "Invalid choice. Please try again.\n");
            return true;
    }

    this->tasksQueue.enqueue(func , this->client_fd, choice);
    std::cout<<"Task Added"<<std::endl;
    return true;
}


void LeaderFollowerDP::promoteFollower(int follower){
    std::string leader = (follower == CONVERSATION) ?
                        "Promotion: Leader-Conversation" : (follower == TASKS) ?
                        "Promotion: Leader-Tasks" : "No-Leader";
    std::cout<<"\n****** "<<leader<<" ******"<<std::endl;
    this->leaderIndex = follower;
    this->cv.notify_all();
}