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

LeaderFollowerDP::LeaderFollowerDP() : leaderIndex(-1), client_fd(-1) {
    this->threadsPool.push_back(std::thread(&LeaderFollowerDP::tasksEnqueing, this));
    this->threadsPool.push_back(std::thread(&LeaderFollowerDP::tasksExecution, this));
    std::cout<<"Created Threades"<<std::endl;
}

LeaderFollowerDP::~LeaderFollowerDP() {
    {
        std::lock_guard<std::mutex> lock(this->mtx);
        this->stop = true;
    }

    tasksQueue.clear();  // Clear the queue
    this->cv.notify_all();  // Notify all threads to stop
    
    // Join all threads
    for(auto& thread : this->threadsPool) {
        if(thread.joinable()) {
            thread.join();
        }
    }
}

void LeaderFollowerDP::promoteFollower(int follower){
    std::cout<<"Follower Has Promoted "<<follower<<std::endl;
    this->leaderIndex = follower;
    this->cv.notify_all();
}


void LeaderFollowerDP::tasksEnqueing(){
    while(true){
        if(this->leaderIndex != CONVERSATION){
            std::unique_lock<std::mutex> lock(this->mtx);
            std::cout<<"Waiting For Promotion"<<std::endl;
            this->cv.wait(lock, [this] { return this->leaderIndex == CONVERSATION; });
        }

        if (stop) {
            std::cout<<"\nServer is Closing..."<<std::endl;
            break;
        }

        if(this->client_fd == -1){
            std::cout<<"\nClient has Exited, No Leader to Promote"<<std::endl;
            promoteFollower(NO_LEADER);
            continue;
        }

        std::cout<<"Starting Conversation...\n"<<std::endl;
        int choice = this->startConversation(this->client_fd);
        if(enqueingChoices(choice)) continue;
        
        std::cout<<"\nExecution Cases Activated"<<std::endl;
        std::cout<<"Promoting Follower to Execute Tasks\n"<<std::endl;
        promoteFollower(TASKS);
    }
}

bool LeaderFollowerDP::enqueingChoices(int choice){
    TaskQueue::TaskType func;
    switch (choice) {
        case 1: // Create graph
            //define task to add to the equeue
            func = [this](int &client_fd, int choice) -> bool {return createGraph(client_fd);};
            this->tasksQueue.enqueue(func , this->client_fd, choice); // add the func to queue with the proper arguments
            break;
        
        case 2: // Add edge
        case 3: // Remove edge
            func = [this](int &client_fd, int choice) -> bool {return modifyGraph(client_fd, choice == 2);};
            this->tasksQueue.enqueue(func , this->client_fd, choice);
            break;

        case 4: // Calculate MST
            func = [this](int &client_fd, int choice) -> bool {return calculateMST(client_fd);};
            this->tasksQueue.enqueue(func , this->client_fd, choice);
            break;

        case 5: // MST operations
        case 6:
        case 7:
        case 8:
        case 9:
            func = [this](int &client_fd, int choice) -> bool {return getMSTData(client_fd, choice);};
            this->tasksQueue.enqueue(func , this->client_fd, choice);
            break;
        
        case 10: // Client Exit
            func = [this](int &client_fd, int choice) -> bool {this->stopClient(client_fd); return true;};
            this->tasksQueue.enqueue(func , this->client_fd, choice);
            return false;
            
        case 0: // Start Executing the tasks
            return false;

        default:
            sendMessage(this->client_fd, "Invalid choice. Please try again.\n");
            return true;
    }

    std::cout<<"Task Added"<<std::endl;
    return true;
}

void LeaderFollowerDP::tasksExecution(){
    while(true){
        if(this->leaderIndex != TASKS){
            std::unique_lock<std::mutex> lock(this->mtx);
            std::cout<<"Waiting For Promotion"<<std::endl;
            this->cv.wait(lock, [this] { return this->leaderIndex == TASKS; });
        }

        if (stop) {
            std::cout<<"\nServer is Closing..."<<std::endl;
            break;
        }

        if(this->client_fd < 0){
            std::cout<<"\nClient has Exited, No Leader to Promote"<<std::endl;
            this->tasksQueue.clear();
            promoteFollower(NO_LEADER);
            continue;
        }

        std::cout<<"Executing Tasks...\n"<<std::endl;
        bool success = this->tasksQueue.executeTask();
        if(!success){
            sendMessage(this->client_fd, "Task execution failed. Make sure to add task according to Healthy Logic.\n");
            std::cout<<"Task Execution Failed, dequeue and continue to next tasks"<<std::endl;
        }

        if(this->tasksQueue.isEmpty()){
            std::cout<<"Tasks Queue is Empty, Promoting Follower"<<std::endl;
            promoteFollower(CONVERSATION);
        }
    }
}

void LeaderFollowerDP::handleRequest(int& client_FD) {
    std::cout<<"Handle Request Connection With A Client"<<std::endl;
    this->client_fd = client_FD;
    promoteFollower(CONVERSATION);

}
