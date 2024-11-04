#ifndef MSTSTRATEGY_HPP
#define MSTSTRATEGY_HPP

#include "Graph.hpp"
#include <memory>
#include <vector>

class MSTStrategy {
public:
    virtual ~MSTStrategy() = default;
    virtual std::unique_ptr<std::vector<std::vector<int>>> computeMST(const Graph& graph) = 0;
};

#endif 
