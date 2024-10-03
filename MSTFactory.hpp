#ifndef MST_FACTORY_HPP
#define MST_FACTORY_HPP

#include "MSTStrategy.hpp"
#include "KruskalStrategy.hpp"
#include "PrimStrategy.hpp"
#include <memory>

class MSTFactory{
public:
    enum AlgorithmType
    {
        Prim,
        Kruskal
    };

public:
    static MSTStrategy* createMSTStrategy(AlgorithmType type) {
        switch (type) {
            case AlgorithmType::Kruskal:
                return new KruskalStrategy();
            case AlgorithmType::Prim:
                return new PrimStrategy();
            default:
                throw std::invalid_argument("Unknown MST Algorithm Type");
        }
    }
};

#endif 
