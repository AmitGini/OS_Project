#ifndef LEADERFOLLOWERDP_HPP
#define LEADERFOLLOWERDP_HPP

#include <functional>
#include <atomic>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "RequestService.hpp"

class LeaderFollowerDP : public RequestService {
    enum class TaskType {
        CREATE_GRAPH = 1,
        ADD_EDGE = 2,
        REMOVE_EDGE = 3,
        COMPUTE_MST = 4,
        GET_MST_DATA = 5,
        EXIT = 10
    };
private:
    std::mutex mtx;
    std::mutex graphMutex;
    std::condition_variable cv;
    std::queue<std::pair<TaskType, std::function<bool()>>> taskQueue;
    bool stop;
    int numThreads;

    // Protects access to the current leader's thread ID
    std::mutex leader_mutex;
    std::thread::id current_leader_id;
    bool leaderExists;

    // Atomic flags to control task execution
    std::atomic<bool> graph_created{false};
    std::atomic<bool> mst_computed{false};
    std::atomic<bool> modified_edges{false}; 

    // Inhertied function to be implemented by derived classes to handle requests
    void handleRequest(int client_FD) override;

    // Promotes the next leader thread
    void promoteNextLeader();

    // Checks if the current thread can become a leader based on the flags
    bool canBecomeLeader();

    // Set the current leader's ID in a thread-safe manner
    void setLeaderID(std::thread::id id);

    // Get the current leader's ID in a thread-safe manner
    std::thread::id getLeaderID();

    // Update the flags based on the completed task type
    void updateFlags(TaskType completedTaskType);

public:
    LeaderFollowerDP();

    // The function that each thread will execute
    void work();

    // Adds a task to the queue
    void addTask(TaskType type, const std::function<bool()>& task);

    // Stops the thread pool
    void stopThreadPool();

    // Task processing function (left empty for customization)
    bool processTask(const std::function<bool()>& task);
};

#endif 
