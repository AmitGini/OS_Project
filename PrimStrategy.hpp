#ifndef PRIMSTRATEGY_HPP
#define PRIMSTRATEGY_HPP

#include "MSTStrategy.hpp"
#include <vector>
#include <limits>
#include <queue>
#include <memory>

class PrimStrategy : public MSTStrategy
{
public:
    std::unique_ptr<std::vector<std::vector<int>>> computeMST(const Graph &graph) override;
};
#endif