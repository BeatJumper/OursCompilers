// ArrayType.h
#pragma once
#include "Type.h"
#include <vector>
#include <numeric>
#include <sstream>

class ArrayType : public Type {
public:
    ArrayType(Type * elementType, std::vector<int> dimensions)
        : Type(Type::ArrayTyID), elementType(elementType), dimensions(dimensions)
    {}

    Type * getElementType() const
    {
        return elementType;
    }
    const std::vector<int> & getDimensions() const
    {
        return dimensions;
    }

    int getTotalElements() const
    {
        if (dimensions.empty())
            return 0;
        return std::accumulate(dimensions.begin(), dimensions.end(), 1, [](int a, int b) { return a * b; });
    }

    virtual int32_t getSize() const override
    {
        if (dimensions.empty())
            return 0;
        return std::accumulate(dimensions.begin(), dimensions.end(), elementType->getSize(), [](int a, int b) {
            return a * b;
        });
    }

    virtual std::string toString() const override
    {
        std::ostringstream oss;
        oss << elementType->toString();
        for (int dim: dimensions) {
            oss << "[" << dim << "]";
        }
        return oss.str();
    }

private:
    Type * elementType;
    std::vector<int> dimensions;
};