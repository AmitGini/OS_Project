#include "Pipeline.hpp"

// Stage definitions
#define STAGE_UNDEFINE -1 // Undefined stage for error handling
#define STAGE_0_START_MST_CALCULATION 0
#define STAGE_1_TOTAL_WEIGHT_MST 1
#define STAGE_2_LON_DIS_BET_TWO_VER_MST 2
#define STAGE_3_SHO_DIS_BET_TWO_VER_MST 3
#define STAGE_4_AVG_DIS_BET_TWO_EDG_MST 4
#define STAGE_5_FINISH_MST_CALCUATION 5

Pipeline::Pipeline()
{
    createAOStages();
    setAONextStage();
}

Pipeline::~Pipeline()
{
    // Lock the mutex to ensure thread safety
    std::lock_guard<std::mutex> lock(pipeMtx);
    for (auto stage : stages)
    {
        stage->stopActiveObject(); // Stop the active object
    }

    // Clear the stages vector to release the shared_ptr resources
    stages.clear();
}

void Pipeline::createAOStages()
{
    try
    {
        // Create stages with error checking
        for (int stageNumber = STAGE_0_START_MST_CALCULATION; stageNumber <= STAGE_5_FINISH_MST_CALCUATION; ++stageNumber)
        {
            std::cout << "***** " << "Creating stage " << stageNumber << " *****" << std::endl;
            stages.push_back(std::make_shared<ActiveObject>(stageNumber)); // Create a shared pointer to an active object
            std::cout << "Stage " << stageNumber << " created successfully" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Stage creation failed: " << e.what() << std::endl;
        throw;
    }
}

void Pipeline::setAONextStage()
{
    try
    {
        std::weak_ptr<ActiveObject> myNextStage = stages[STAGE_1_TOTAL_WEIGHT_MST]; // Stage 1
        stages[STAGE_0_START_MST_CALCULATION]->setNextStage(myNextStage);           // Set Stage 0 -> 1
        myNextStage = stages[STAGE_2_LON_DIS_BET_TWO_VER_MST];                      // Stage 2
        stages[STAGE_1_TOTAL_WEIGHT_MST]->setNextStage(myNextStage);                // Set Stage 1 -> 2
        myNextStage = stages[STAGE_3_SHO_DIS_BET_TWO_VER_MST];                      // Stage 3
        stages[STAGE_2_LON_DIS_BET_TWO_VER_MST]->setNextStage(myNextStage);         // Set Stage 2 -> 3
        myNextStage = stages[STAGE_4_AVG_DIS_BET_TWO_EDG_MST];                      // Stage 4
        stages[STAGE_3_SHO_DIS_BET_TWO_VER_MST]->setNextStage(myNextStage);         // Set Stage 3 -> 4
        myNextStage = stages[STAGE_5_FINISH_MST_CALCUATION];                        // Stage 5
        stages[STAGE_4_AVG_DIS_BET_TWO_EDG_MST]->setNextStage(myNextStage);         // Set Stage 4 -> 5
    }
    catch (const std::exception &e)
    {
        std::cerr << "Set Next Active Object Failed: " << e.what() << std::endl;
        throw;
    }
}

// define the task handlers for each stage
void Pipeline::setTaskHandler()
{
    try
    {
        // Define task handlers for each stage
        stages[STAGE_0_START_MST_CALCULATION]->setTaskHandler([this](std::weak_ptr<Graph> graph) -> void
                                                              {
            // Lock the weak_ptr to get a shared_ptr (to be able to access the graph methods)
            std::shared_ptr<Graph> sharedGraph = graph.lock();

            // Check if the shared_ptr is valid
            if (sharedGraph) {
                sharedGraph->setMSTDataCalculationNextStatus();  // Access the method  (next set status is progress = 0 previous is none = -1)
            } else {   // Handle the case where the managed object no longer exists
                std::cerr << "Graph object no longer exists." << std::endl;
            } });
        stages[STAGE_1_TOTAL_WEIGHT_MST]->setTaskHandler([this](std::weak_ptr<Graph> graph) -> void
                                                         {
            // Lock the weak_ptr to get a shared_ptr (to be able to access the graph methods)
            std::shared_ptr<Graph> sharedGraph = graph.lock();

            // Check if the shared_ptr is valid
            if (sharedGraph) {
                sharedGraph->setMSTTotalWeight();  // Access the method
            } else {   // Handle the case where the managed object no longer exists
                std::cerr << "Graph object no longer exists." << std::endl;
            } });
        stages[STAGE_2_LON_DIS_BET_TWO_VER_MST]->setTaskHandler([this](std::weak_ptr<Graph> graph) -> void
                                                                {
            // Lock the weak_ptr to get a shared_ptr (to be able to access the graph methods)
            std::shared_ptr<Graph> sharedGraph = graph.lock();

            // Check if the shared_ptr is valid
            if (sharedGraph) {
                sharedGraph->setMSTLongestDistance();  // Access the method
            } else {   // Handle the case where the managed object no longer exists
                std::cerr << "Graph object no longer exists." << std::endl;
            } });
        stages[STAGE_3_SHO_DIS_BET_TWO_VER_MST]->setTaskHandler([this](std::weak_ptr<Graph> graph) -> void
                                                                {
            // Lock the weak_ptr to get a shared_ptr (to be able to access the graph methods)
            std::shared_ptr<Graph> sharedGraph = graph.lock();

            // Check if the shared_ptr is valid
            if (sharedGraph) {
                sharedGraph->setMSTShortestDistance();  // Access the method
            } else {   // Handle the case where the managed object no longer exists
                std::cerr << "Graph object no longer exists." << std::endl;
            } });
        stages[STAGE_4_AVG_DIS_BET_TWO_EDG_MST]->setTaskHandler([this](std::weak_ptr<Graph> graph) -> void
                                                                {
            // Lock the weak_ptr to get a shared_ptr (to be able to access the graph methods)
            std::shared_ptr<Graph> sharedGraph = graph.lock();

            // Check if the shared_ptr is valid
            if (sharedGraph) {
                sharedGraph->setMSTAvgEdgeWeight();  // Access the method
            } else {   // Handle the case where the managed object no longer exists
                std::cerr << "Graph object no longer exists." << std::endl;
            } });
        stages[STAGE_5_FINISH_MST_CALCUATION]->setTaskHandler([this](std::weak_ptr<Graph> graph) -> void
                                                              {
            // Lock the weak_ptr to get a shared_ptr (to be able to access the graph methods)
            std::shared_ptr<Graph> sharedGraph = graph.lock();

            // Check if the shared_ptr is valid
            if (sharedGraph) {
                sharedGraph->setMSTDataCalculationNextStatus();  // Access the method (next set status is finish = 1, previous one is progress = 0)
            } else {   // Handle the case where the managed object no longer exists
                std::cerr << "Graph object no longer exists." << std::endl;
            } });
    }
    catch (const std::exception &e)
    {
        std::cerr << "Set Next Active Object Failed: " << e.what() << std::endl;
        throw;
    }
}

void Pipeline::processGraphs(std::vector<std::weak_ptr<Graph>> &graphs)
{
    // Lock the mutex to ensure thread safety
    std::lock_guard<std::mutex> lock(this->pipeMtx);
    std::shared_ptr<Graph> sharedGraph;
    for (auto graph : graphs)
    {
        sharedGraph = graph.lock(); // to use graph methods we need to lock the weak_ptr to get a shared_ptr
        // Check if the graph already has calculated the MST data - if not, enqueue it to the first stage
        if (sharedGraph->getMSTDataStatusCalculation() < STAGE_0_START_MST_CALCULATION)
        {
            stages[STAGE_1_TOTAL_WEIGHT_MST]->enqueueTask(graph); // enqueue the weak ptr although we have used shared ptr for validation
        }
    }
}