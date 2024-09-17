#ifndef MST_FACTORY_HPP
#define MST_FACTORY_HPP

#include "MSTStrategy.hpp"
#include <memory>

class MSTFactory
{
public:
    enum Algorithm
    {
        PRIM,
        KRUSKAL
    };

    static std::unique_ptr<MSTStrategy> createMSTAlgorithm(Algorithm algo)
    {
        switch (algo)
        {
        case PRIM:
            return std::make_unique<PrimMST>();
        case KRUSKAL:
            return std::make_unique<KruskalMST>();
        default:
            throw std::invalid_argument("Unknown algorithm");
        }
    }
};

#endif 
