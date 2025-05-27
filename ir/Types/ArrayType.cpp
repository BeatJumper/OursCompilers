#include "ArrayType.h"
#include <sstream>
#include <numeric>
#include <functional>

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
    return std::accumulate(dimensions.begin(), dimensions.end(), 1, std::multiplies<int>());
}

int32_t ArrayType::getSize() const
{
    if (dimensions.empty())
        return 0;
    // 总元素个数乘以单个元素大小
    return getTotalElements() * elementType->getSize();
}

std::string ArrayType::toString() const
{
    if (dimensions.empty()) {
        return elementType->toString();
    }

    std::string result = elementType->toString();

    // 生成LLVM IR标准格式 [N x type]
    // 从最内层开始构建，生成嵌套的数组类型
    for (int i = dimensions.size() - 1; i >= 0; i--) {
        result = "[" + std::to_string(dimensions[i]) + " x " + result + "]";
    }

    return result;
}

bool ArrayType::isArrayType() const
{
    return true;
}