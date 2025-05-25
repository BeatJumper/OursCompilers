#include "Types/ArrayType.h"
#include <sstream>
#include <numeric>

ArrayType::ArrayType(Type * elementType, std::vector<int> dimensions)
    : Type(Type::ArrayTyID), elementType(elementType), dimensions(dimensions)
{}

Type * ArrayType::getElementType() const
{
    return elementType;
}

const std::vector<int> & ArrayType::getDimensions() const
{
    return dimensions;
}

int ArrayType::getTotalElements() const
{
    if (dimensions.empty())
        return 0;
    return std::accumulate(dimensions.begin(), dimensions.end(), 1, [](int a, int b) { return a * b; });
}

int32_t ArrayType::getSize() const
{
    if (dimensions.empty())
        return 0;
    return std::accumulate(dimensions.begin(), dimensions.end(), elementType->getSize(), [](int a, int b) {
        return a * b;
    });
}

std::string ArrayType::toString() const
{
    std::ostringstream oss;
    oss << elementType->toString();
    for (int dim: dimensions) {
        oss << "[" << dim << "]";
    }
    return oss.str();
}