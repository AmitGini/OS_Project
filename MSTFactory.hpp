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

    static std::unique_ptr<MSTStrategy> createMSTStrategy(AlgorithmType type) {
        switch (type) {
            case AlgorithmType::Kruskal:
                return std::make_unique<KruskalStrategy>();
            case AlgorithmType::Prim:
                return std::make_unique<PrimStrategy>();
            default:
                throw std::invalid_argument("Unknown MST Algorithm Type");
        }
    }
};

#endif 
