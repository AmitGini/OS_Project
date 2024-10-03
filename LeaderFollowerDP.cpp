#include "LeaderFollowerDP.hpp"

LeaderFollowerDP::LeaderFollowerDP() 
        : stop(false), num_threads(std::thread::hardware_concurrency()), current_leader(-1) {
        for (int i = 0; i < num_threads; ++i) {
            t_tasks.emplace_back(&LeaderFollowerDP::workerThread, this, i);
        }
}

LeaderFollowerDP::~LeaderFollowerDP() {
    stop = true;
    cv.notify_all();
    for (auto& thread : t_tasks) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void LeaderFollowerDP::handleRequest(int client_FD) {
    int choice;
    while (true) {
        choice = startConversation(client_FD);
        std::cout << "Choice: " << choice << std::endl;

        if (choice == static_cast<int>(TaskType::EXIT)) {
            sendMessage(client_FD, "Goodbye!");
            close(client_FD);
            return;
        }

        TaskType taskType = static_cast<TaskType>(choice);
        int priority = getPriority(taskType);

        {
            std::lock_guard<std::mutex> lock(graphMutex);
            tasks.push({client_FD, taskType, priority, choice});
        }
        
        if (current_leader == -1) {
            current_leader = 0;
        }
        cv.notify_all();
    }
}

void LeaderFollowerDP::workerThread(int thread_id){
    while (!stop) {
        std::unique_lock<std::mutex> lock(thread_id);
        cv.wait(lock, [this, thread_id] { 
            return (thread_id == current_leader && !tasks.empty()) || stop; 
        });

        if (stop && tasks.empty()) {
            return;
        }

        if (thread_id == current_leader && !tasks.empty()) {
            Task task = tasks.top();
            tasks.pop();
        
            processTask(task);
            promoteNextLeader();
        }
    }
}

void LeaderFollowerDP::promoteNextLeader() {
    std::lock_guard<std::mutex> lock(graphMutex);
    current_leader = (current_leader + 1) % num_threads;
    cv.notify_all();
}

void LeaderFollowerDP::processTask(const Task& task) {
    std::lock_guard<std::mutex> lock(graphMutex);
    switch (task.type) {
        case TaskType::CREATE_GRAPH:
            if (createGraph(task.client_FD)) {
                graph_created = true;
                modified_edges.clear();
                mst_computed = false;
                std::cout << "Graph created. Task Stage 1 complete." << std::endl;
            }
            break;

        case TaskType::ADD_EDGE:
        case TaskType::REMOVE_EDGE:
            if (!graph_created) {
                sendMessage(task.client_FD, "Graph not created yet. Please create a graph first.\n");
                return;
            }
            if (modifyGraph(task.client_FD, task.type == TaskType::ADD_EDGE)) {
                modified_edges.insert(task.client_FD); // Use a unique identifier for the edge
                mst_computed = false;
                std::cout << "Graph modified. Task Stage 2 complete." << std::endl;
            }
            break;

        case TaskType::COMPUTE_MST:
            if (!graph_created) {
                sendMessage(task.client_FD, "Graph not created yet. Please create a graph first.\n");
                return;
            }
            if (modified_edges.empty()) {
                sendMessage(task.client_FD, "No edges to compute MST. Please add some edges first.\n");
                return;
            }
            if (calculateMST(task.client_FD)) {
                mst_computed = true;
                std::cout << "MST computed. Task Stage 3 complete." << std::endl;
            }
            break;

        case TaskType::GET_MST_DATA:
            if (!mst_computed) {
                sendMessage(task.client_FD, "MST not computed yet. Please compute the MST first.\n");
                return;
            }
            getMSTData(task.client_FD, task.choice);
            std::cout << "MST data retrieved. Task Stage 4 complete." << std::endl;
            break;
    }
}

int LeaderFollowerDP::getPriority(TaskType type) {
    switch (type) {
        case TaskType::CREATE_GRAPH:
            return 4;
        case TaskType::ADD_EDGE:
        case TaskType::REMOVE_EDGE:
            return 3;
        case TaskType::COMPUTE_MST:
            return 2;
        case TaskType::GET_MST_DATA:
            return 1;
        default:
            return 0;
    }
}
