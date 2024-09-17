#ifndef MSTSTRATEGY_HPP
#define MSTSTRATEGY_HPP

#include "Graph.hpp"
#include <vector>

class MSTStrategy
{
public:
    virtual void computeMST(const Graph &graph) = 0;
    virtual int getTotalWeight() const = 0;
    virtual int getLongestDistance() const = 0;
    virtual double getAverageDistance() const = 0;
    virtual int getShortestDistance() const = 0;
};

#endif 
