#ifndef MSTSTRATEGY_HPP
#define MSTSTRATEGY_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <iostream>

class MSTStrategy
{
public:
    std::mutex cout_mtx;
    virtual ~MSTStrategy() = default;
    virtual std::unique_ptr<std::vector<std::vector<int>>> computeMST(const std::vector<std::vector<int>> &graphAdjacencyMatrix) = 0;
};

#endif
