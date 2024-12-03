#ifndef KRUSKALSTRATEGY_HPP
#define KRUSKALSTRATEGY_HPP

#include "MSTStrategy.hpp"
#include <vector>
#include <tuple>
#include <algorithm>
#include <memory>

class KruskalStrategy : public MSTStrategy
{
public:
    std::unique_ptr<std::vector<std::vector<int>>> computeMST(const std::vector<std::vector<int>> &graphAdjacencyMatrix) override;

private:
    bool hasCycle(int current, int parent, const std::vector<std::vector<int>> &adj, std::vector<bool> &visited);
};
#endif
