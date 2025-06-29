///
/// @file GlobalVariable.cpp
/// @brief 全局变量实现
///

#include "GlobalVariable.h"
#include "ArrayType.h"
#include "ConstInt.h"
#include "ConstFloat.h"

/// @brief 格式化数组初始化器为正确的嵌套结构
std::pair<std::string, int>
GlobalVariable::formatArrayInitializer(ArrayType * arrayType, const std::vector<Value *> & values, int startIndex)
{
    std::string result;
    int consumedElements = 0;

    if (!arrayType) {
        return {"", 0};
    }

    const std::vector<int> & dimensions = arrayType->getDimensions();
    if (dimensions.empty()) {
        return {"", 0};
    }

    int outerSize = dimensions[0];
    Type * elementType = arrayType->getElementType();

    printf("Debug: formatArrayInitializer - arrayType: %s, outerSize: %d, startIndex: %d, values.size(): %zu\n",
           arrayType->toString().c_str(),
           outerSize,
           startIndex,
           values.size());

    // 打印前几个值用于调试
    for (size_t i = startIndex; i < values.size() && i < startIndex + 8; ++i) {
        if (auto constInt = dynamic_cast<ConstInt *>(values[i])) {
            printf("  values[%zu] = %d\n", i, constInt->getVal());
        }
    }

    result += "[";

    if (elementType->isArrayType()) {
        // 多维数组：递归处理每个子数组
        ArrayType * nestedArrayType = static_cast<ArrayType *>(elementType);
        for (int i = 0; i < outerSize; ++i) {
            if (i > 0)
                result += ", ";

            auto nestedResult = formatArrayInitializer(nestedArrayType, values, startIndex + consumedElements);
            result += nestedArrayType->toString() + " " + nestedResult.first;
            consumedElements += nestedResult.second;
        }
    } else {
        // 一维数组：直接输出元素
        int elementsPerRow = outerSize;
        for (int i = 0; i < elementsPerRow && (startIndex + consumedElements) < static_cast<int>(values.size()); ++i) {
            if (i > 0)
                result += ", ";
            result += formatValue(values[startIndex + consumedElements]);
            consumedElements++;
        }

        // 如果元素不足，用零填充
        while (consumedElements < elementsPerRow) {
            if (consumedElements > 0)
                result += ", ";
            if (elementType->isIntegerType()) {
                result += "i32 0";
            } else if (elementType->isFloatType()) {
                result += "float 0.0";
            } else {
                result += "i32 0";
            }
            consumedElements++;
        }
    }

    result += "]";
    return {result, consumedElements};
}

/// @brief 格式化单个值
std::string GlobalVariable::formatValue(Value * value)
{
    if (ConstInt * constInt = dynamic_cast<ConstInt *>(value)) {
        return "i32 " + std::to_string(constInt->getVal());
    } else if (ConstFloat * constFloat = dynamic_cast<ConstFloat *>(value)) {
        return "float " + constFloat->getIRName();
    } else if (GlobalVariable * globalVar = dynamic_cast<GlobalVariable *>(value)) {
        return expandGlobalVariableContent(globalVar);
    } else {
        return "i32 0";
    }
}
