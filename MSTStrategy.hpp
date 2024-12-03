#ifndef MSTSTRATEGY_HPP
#define MSTSTRATEGY_HPP

#include <memory>
#include <vector>

class MSTStrategy
{
public:
    virtual ~MSTStrategy() = default;
    virtual std::unique_ptr<std::vector<std::vector<int>>> computeMST(const std::vector<std::vector<int>> &graphAdjacencyMatrix) = 0;
};

#endif
