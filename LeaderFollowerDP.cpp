#include "LeaderFollowerDP.hpp"

#define NO_PROCESS -1
#define AT_PROCESS 0
#define FINISH_PROCESS 1

// Constructor
LeaderFollowerDP::LeaderFollowerDP() : stop(false)
{
    try
    {
        int myLuckNumber = 4;
        for (int i = 0; i < myLuckNumber; i++)
        {
            this->threadsPool.push_back(std::make_unique<std::thread(&LeaderFollowerDP::work, this)>); // Create the first thread for the leader to handle the conversation and enqueue tasks
        }
        std::cout << "Created Threades" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// TODO
LeaderFollowerDP::~LeaderFollowerDP()
{
    std::cout << "**** Closing Pattern Type ****" << std::endl;
    std::cout << "Deleting Leader-Follower" << std::endl;

    std::cout << "Leader Follower Threads has Closed!" << std::endl;
}

void LeaderFollowerDP::processGraphs(std::vector<std::weak_ptr<Graph>> &graphs)
{
    // Lock the mutex to ensure thread safety
    std::lock_guard<std::mutex> lock(this->mtx_lf);
    std::shared_ptr<Graph> sharedGraph;
    for (auto graph : graphs)
    {
        sharedGraph = graph.lock(); // to use graph methods we need to lock the weak_ptr to get a shared_ptr
        // Check if the graph already has calculated the MST data - if not, enqueue it to the first stage
        if (sharedGraph->getMSTDataStatusCalculation() < AT_PROCESS)
        {
            this->queue_taskData.push(graph); // enqueue the weak ptr although we have used shared ptr for validation
        }
    }
    this->cv_lf.notify_one(); // Notify all the threads to wake up and start working
}

// Start the conversation with the client
void LeaderFollowerDP::work()
{
    while (!stop)
    {
        if (iAmNotLeader())
        { // Check if the leader index is not the conversation leader
            std::unique_lock<std::mutex> lock(this->mtx_lf);
            std::cout << "** Conversation Follower ** - Waiting For Promotion" << std::endl;
            this->cv_lf.wait(lock, [this]
                             { return (this->leaderIndex == CONVERSATION || this->stop); }); // Wait for the leader index to be the conversation leader or the stop flag to be set
        }

        if (stop)
            break; // If the stop flag is set, break the loop and exit the thread

        if (this->client_fd < 0)
        { // Check if the client file descriptor is less than 0 (client has disconnected)
            std::cout << "\n** Client has Left **\n"
                      << std::endl;
            promoteFollower(); // Promote the follower to leader
            continue;          // Continue to the next iteration of the loop
        }

        // Start the conversation with the client
        try
        {
            if (this->toCloseClient)
            {                               // Check if the client has disconnected
                promoteFollower(NO_LEADER); // Promote the follower to leader
            }
            else
            { // If the client is connected
                std::cout << "Starting Conversation with Client: " << client_fd << std::endl;
                int choice = this->startConversation(this->client_fd);
                // If return false - client choice is 0 to execute tasks or 10 to exit
                if (!enqueingChoices(choice))
                { // If the choice is to execute tasks or exit
                    std::cout << "\nPromoting Follower to Execute Tasks\n"
                              << std::endl;
                    promoteFollower(TASKS); // Promote the follower to leader to execute the tasks
                }
            }
        }
        catch (const std::exception &e)
        {
            int follower = (client_fd < 0) ? NO_LEADER : CONVERSATION;
            promoteFollower(follower);
        }
    }
}

bool LeaderFollower::amIALeader()
{
    return this->leaderIndex != CONVERSATION;
}
// Execute the tasks
void LeaderFollowerDP::tasksExecution()
{
    while (true)
    {
        if (this->leaderIndex != TASKS)
        {
            std::unique_lock<std::mutex> lock(this->mtx_lf);
            std::cout << "** Task Follower ** - Waiting For Promotion" << std::endl;
            this->cv_lf.wait(lock, [this]
                             { return this->leaderIndex == TASKS || this->stop; }); // Wait for the leader index to be the task leader or the stop flag to be set
        }

        if (stop)
            break; // If the stop flag is set, break the loop and exit the thread

        if (this->client_fd < 0)
        {
            std::cout << "\n** Client has Left **\n"
                      << std::endl;
            this->toCloseClient = true;
            promoteFollower(NO_LEADER);
            continue;
        }

        try
        {
            if (this->toCloseClient)
            {
                promoteFollower(NO_LEADER);
            }
            else if (this->tasksQueue.isEmpty())
            {
                std::cout << "Tasks Queue is Empty" << std::endl;
                promoteFollower(CONVERSATION);
            }
            else
            {
                bool success = this->tasksQueue.executeTask();
                if (!success)
                {
                    sendMessage(this->client_fd, "Task execution failed. Try to use Healthy Logic. (Recommended)\n");
                    std::cout << "*** Task Execution Failed - Dequeue and Keep Working ***" << std::endl;
                }
            }
        }
        catch (const std::exception &e)
        {
            int follower = (client_fd < 0) ? NO_LEADER : TASKS;
            promoteFollower(follower);
        }
    }
}

// Enqueue the choices
bool LeaderFollowerDP::enqueingChoices(int choice)
{
    TaskQueue::TaskType func;
    std::string message;

    switch (choice)
    {
    case 1: // Create graph
        // define task to add to the equeue
        func = [this](int &client_fd, int choice) -> bool
        { return createGraph(client_fd); };
        break;

    case 2: // Add edge
    case 3: // Remove edge
        func = [this](int &client_fd, int choice) -> bool
        { return modifyGraph(client_fd, choice == 2); };
        break;

    case 4: // Calculate MST
        func = [this](int &client_fd, int choice) -> bool
        { return calculateMST(client_fd); };
        break;

    case 5: // MST operations
    case 6:
    case 7:
    case 8:
    case 9:
        func = [this](int &client_fd, int choice) -> bool
        { return getMSTData(client_fd, choice); };
        break;

    case 10: // Client Exit
        func = [this](int &client_fd, int choice) -> bool
        {
                this->toCloseClient = true;
                return this->stopClient(client_fd); };

        this->tasksQueue.clear(); // Clear the queue of tasks client might entered
        this->tasksQueue.enqueue(func, this->client_fd, choice);
        return false; // return value false execute the tasks

    case 0:           // Start Executing the tasks
        return false; // return value false to execute the tasks
    default:          // Invalid choice - return to conversation
        return true;  // return value true to continue the conversation
    }

    this->tasksQueue.enqueue(func, this->client_fd, choice);
    std::cout << "Task Added" << std::endl;
    return true;
}

// Promote the follower to leader
void LeaderFollowerDP::promoteFollower(int follower)
{
    std::string leader = (follower == CONVERSATION) ? "Promotion: Leader-Conversation" : (follower == TASKS) ? "Promotion: Leader-Tasks"
                                                                                                             : "No-Leader";
    std::cout << "\n****** " << leader << " ******" << std::endl;
    this->leaderIndex = follower;
    this->cv_lf.notify_all(); // Notify all the threads to wake up and start working
}