#ifndef MSTSTRATEGY_HPP
#define MSTSTRATEGY_HPP

#include "Graph.hpp"
#include <vector>

class MSTStrategy{
public:
    virtual std::vector<std::vector<int>>* computeMST(const Graph &graph) = 0;
};

#endif 
