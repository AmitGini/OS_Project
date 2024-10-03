#ifndef LEADERFOLLOWERDP_HPP
#define LEADERFOLLOWERDP_HPP
#include "RequestService.hpp"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <unordered_set>

class LeaderFollowerDP : public RequestService {
public:
    enum class TaskType {
        CREATE_GRAPH = 1,
        ADD_EDGE = 2,
        REMOVE_EDGE = 3,
        COMPUTE_MST = 4,
        GET_MST_DATA = 5,
        EXIT = 10
    };

    struct Task {
        int client_FD;
        TaskType type;
        int priority;
        int choice;

        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };

    LeaderFollowerDP();
    ~LeaderFollowerDP() override;
    void handleRequest(int client_FD) override;

private:


    std::atomic<bool> stop;
    int num_threads;
    std::atomic<int> current_leader;
    std::vector<std::thread> t_tasks;
    std::vector<std::mutex> t_mutexes; 
    std::priority_queue<Task> tasks;
    std::mutex graphMutex;
    std::condition_variable cv;

    std::atomic<bool> graph_created{false};
    std::unordered_set<int> modified_edges;
    std::atomic<bool> mst_computed{false};

    void workerThread(int thread_id);
    void promoteNextLeader();
    void processTask(const Task& task);
    int getPriority(TaskType type);
};
#endif