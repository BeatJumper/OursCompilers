#pragma once
#include "Type.h"
#include <vector>

class ArrayType : public Type {
public:
    /// @brief 构造函数
    /// @param elementType 元素类型
    /// @param dimensions 各维度大小
    ArrayType(Type * elementType, std::vector<int> dimensions);

    /// @brief 获取元素类型
    /// @return 元素类型指针
    Type * getElementType() const;

    /// @brief 获取维度信息
    /// @return 维度向量的常量引用
    const std::vector<int> & getDimensions() const;

    /// @brief 获取数组总元素个数
    /// @return 总元素个数
    int getTotalElements() const;

    /// @brief 获取数组占用的总字节数
    /// @return 字节数
    virtual int32_t getSize() const override;

    /// @brief 转换为LLVM IR格式的字符串
    /// @return LLVM IR格式的字符串，如 "[5 x i32]" 或 "[3 x [4 x i32]]"
    virtual std::string toString() const override;

    /// @brief 检查是否是数组类型
    /// @return 总是返回true
    virtual bool isArrayType() const override;

private:
    Type * elementType;          ///< 数组元素类型
    std::vector<int> dimensions; ///< 各维度大小
};