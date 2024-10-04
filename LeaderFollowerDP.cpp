#include "LeaderFollowerDP.hpp"
#include <iostream>
#include <thread>
#include <chrono>

LeaderFollowerDP::LeaderFollowerDP(

) : leaderExists(false), stop(false), numThreads(0) {
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 2; // Fallback to 2 threads if hardware concurrency cannot be determined
    }
    this->numThreads = num_threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        std::thread(&LeaderFollowerDP::work, this).detach();
    }
    std::cout << "Started " << num_threads << " threads based on hardware concurrency.\n";
}

void LeaderFollowerDP::promoteNextLeader() {
    std::unique_lock<std::mutex> lock(mtx);
    leaderExists = false; // Release leadership
    cv.notify_one(); // Notify waiting threads for the next leader
}

bool LeaderFollowerDP::canBecomeLeader() {
    // This method is called with mtx already locked
    if (taskQueue.empty()) return false;
    
    TaskType taskType = taskQueue.front().first;
    
    switch(taskType) {
        case TaskType::CREATE_GRAPH:
            return true;
        case TaskType::ADD_EDGE:
        case TaskType::REMOVE_EDGE:
            return graph_created;
        case TaskType::COMPUTE_MST:
            return graph_created && modified_edges;
        case TaskType::GET_MST_DATA:
            return mst_computed;
        default:
            return false;
    }
}


void LeaderFollowerDP::work() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (taskQueue.empty()) {
            std::cout << "Thread " << std::this_thread::get_id() << " waiting for task\n";
        }
        
        cv.wait(lock, [this]() { return stop || (!taskQueue.empty() && !leaderExists);});
        if (stop) break;

        if (taskQueue.empty() || leaderExists) {
            std::cout << "Thread " << std::this_thread::get_id() 
                      << " woke up but conditions not met\n";
            continue;
        }

        if (!canBecomeLeader()) {
            std::cout << "Thread " << std::this_thread::get_id() 
                      << " cannot handle task type, Deleting Task!" 
                      << static_cast<int>(taskQueue.front().first) << "\n";
            taskQueue.pop();
            continue;
        }

        leaderExists = true;
        setLeaderID(std::this_thread::get_id());

        std::cout << "Thread " << getLeaderID() 
                  << " became leader for task type " 
                  << static_cast<int>(taskQueue.front().first) << "\n";

        auto currentTask = taskQueue.front();
        taskQueue.pop();
        
        bool taskSuccess = processTask(currentTask.second);
        
        if (!taskSuccess) {
            std::cout<<"Task Failed"<<std::endl;
        } else {
            std::cout<<"Task Completed"<<std::endl;
            updateFlags(currentTask.first);
        }
        
        lock.unlock();
        promoteNextLeader();
    }
}

void LeaderFollowerDP::updateFlags(TaskType completedTaskType) {
    switch(completedTaskType) {
        case TaskType::CREATE_GRAPH:
            graph_created = true;
            modified_edges = false;
            mst_computed = false;
            break;
        case TaskType::ADD_EDGE:
        case TaskType::REMOVE_EDGE:
            modified_edges = true;
            mst_computed = false;
            break;
        case TaskType::COMPUTE_MST:
            mst_computed = true;
            break;
        default:
            break;
    }
}

void LeaderFollowerDP::addTask(TaskType type, const std::function<bool()>& task) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        taskQueue.push({type, task});
        std::cout<<"Adding Task: "<<(int)type<<", And Task Queue Size: "<<taskQueue.size()<<std::endl;
    }
    cv.notify_one(); // Notify threads about a new task
    std::cout<<"Notify Next Thread"<<std::endl;
}

void LeaderFollowerDP::stopThreadPool() {
    std::unique_lock<std::mutex> lock(mtx);
    stop = true;
    cv.notify_all(); // Wake up all threads to finish execution
}

bool LeaderFollowerDP::processTask(const std::function<bool()>& task) {
    try {
        return task();
    } catch (const std::exception& e) {
        std::cerr << "Task failed with exception: " << e.what() << std::endl;
        return false;
    }
}

void LeaderFollowerDP::setLeaderID(std::thread::id id) {
    std::lock_guard<std::mutex> lock(leader_mutex);
    current_leader_id = id;
}

std::thread::id LeaderFollowerDP::getLeaderID() {
    std::lock_guard<std::mutex> lock(leader_mutex);
    return current_leader_id;
}

void LeaderFollowerDP::handleRequest(int client_FD) {
    while (true) {
        
        while(leaderExists){
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]() {
            return stop || taskQueue.empty() || !leaderExists;
            });
        } 

        int choice = startConversation(client_FD);
        int mstDataChoice = 0;
        if (choice == static_cast<int>(TaskType::EXIT)) {
            sendMessage(client_FD, "Goodbye!");
            close(client_FD);
            return;
        }
        if(choice >= 5 && choice <= 9){
            mstDataChoice = choice;
            choice = 5;
        }

        TaskType taskType = static_cast<TaskType>(choice);
        
        std::function<bool()> task;
        {
            std::lock_guard<std::mutex> lock(graphMutex);
            switch(taskType) {
                case TaskType::CREATE_GRAPH:
                    task = [this, client_FD]() { 
                        std::lock_guard<std::mutex> lockLeader(leader_mutex);
                        std::lock_guard<std::mutex> lockGraph(graphMutex);
                        if(createGraph(client_FD)){
                            std::cout<<"Graph Created"<<std::endl;
                            return true;
                        }
                        return false;
                    };
                    break;
                case TaskType::ADD_EDGE:
                    task = [this, client_FD]() { 
                        std::lock_guard<std::mutex> lockLeader(leader_mutex);
                        std::lock_guard<std::mutex> lock(graphMutex);
                        if(modifyGraph(client_FD, true)){
                            std::cout<<"Edge Added"<<std::endl;
                            return true;
                        }
                        return false;
                    };
                    break;
                case TaskType::REMOVE_EDGE:
                    task = [this, client_FD]() { 
                        std::lock_guard<std::mutex> lockLeader(leader_mutex);
                        std::lock_guard<std::mutex> lock(graphMutex);
                        if(modifyGraph(client_FD, true)){
                            std::cout<<"Edge Added"<<std::endl;
                            return true;
                        }
                        return false;
                    };
                case TaskType::COMPUTE_MST:
                    task = [this, client_FD]() { 
                        std::lock_guard<std::mutex> lockLeader(leader_mutex);
                        std::lock_guard<std::mutex> lock(graphMutex);
                        if(calculateMST(client_FD)){
                            std::cout<<"MST Calculated"<<std::endl;
                            return true;
                        }
                        return false;
                    };
                    break;
                case TaskType::GET_MST_DATA:
                    task = [this, client_FD, mstDataChoice]() {
                        std::lock_guard<std::mutex> lockLeader(leader_mutex);
                        std::lock_guard<std::mutex> lock(graphMutex);
                        getMSTData(client_FD, mstDataChoice);
                        return true;
                    };
                    break;
                default:
                    continue;
            }
        }

        addTask(taskType, task);
        std::cout<<choice<<" Task Added"<<std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
