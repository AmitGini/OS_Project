#ifndef PRIMSTRATEGY_HPP
#define PRIMSTRATEGY_HPP

#include "MSTStrategy.hpp"
#include "Graph.hpp"
#include <vector>
#include <limits>
#include <queue>

class PrimStrategy : public MSTStrategy {
public:
    std::vector<std::vector<int>>* computeMST(const Graph &graph) override;
};

#endif 