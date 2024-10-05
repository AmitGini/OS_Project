#include "ActiveObjectDP.hpp"

#define STAGE1 1
#define STAGE2 2
#define STAGE3 3
#define STAGE4 4

ActiveObjectDP::ActiveObjectDP(){
    instanceNumber = ++instanceCount;
    metConditionForNextStage = false;
    done = false;
    worker = std::thread(&ActiveObjectDP::work, this);
}

ActiveObjectDP::~ActiveObjectDP(){
    {
        std::unique_lock<std::mutex> lock(mutex);
        done = true;
    }
    condition.notify_one();
    if (worker.joinable()) {
        worker.join();
    }
    std::cout << "Stage "<< instanceNumber <<": ActiveObject destroyed and worker thread joined." << std::endl;
}

void ActiveObjectDP::addTask(std::function<bool()> task) {
    if(tasks.empty()){
        std::unique_lock<std::mutex> lock(mutex);
        tasks.push(std::move(task));
        std::cout << "Stage "<< instanceNumber <<": Task added. Total tasks: " << tasks.size() << std::endl;
        condition.notify_one();
    }
}


bool ActiveObjectDP::waitForCompletion() {
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [this] { return tasks.empty() && metConditionForNextStage; });
    return true;
}

bool ActiveObjectDP::getMetConditionForNextStage() const{
    return metConditionForNextStage;
}

void ActiveObjectDP::setMetConditionForNextStage(bool status){
    metConditionForNextStage = status;
}

void ActiveObjectDP::work() {
    std::function<bool()> task;
    
    while (!done) {
        std::unique_lock<std::mutex> lock(mutex);
        if(tasks.empty()) {
            condition.wait(lock, [this] { return !tasks.empty() || done; });
        }
        if (done && tasks.empty()) {
            std::cout << "Stage "<<instanceNumber<<": Worker thread exiting." << std::endl;
            return;
        }
        task = std::move(tasks.front());

        try {
            
            std::cout << "Stage "<<instanceNumber<<": Task started. Remaining tasks: " << tasks.size() << std::endl;
            if(task()){
                std::cout << "Stage "<<instanceNumber<<": Task completed successfully." << std::endl;
                tasks.pop();
            }else throw std::exception();
        } catch (const std::exception& e) {
            std::cerr << "Stage "<<instanceNumber<<": Delete Task, Exception in task execution: " << e.what() << std::endl;
            tasks.pop();
        }
        
        if(tasks.empty()) {
            std::cout<<"Stage "<<instanceNumber<<": All tasks completed for instance "<<instanceNumber<<"."<<std::endl;
            std::cout<<"Stage "<<instanceNumber<<": Notify all - Next"<<std::endl;
            condition.notify_all();
        }else {
            std::cout<<"Stage "<<instanceNumber<<": Notify Next task in this Stage"<<std::endl;
            condition.notify_one();
        }
    }
}