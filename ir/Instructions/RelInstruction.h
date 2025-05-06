///
/// @file RelInstruction.h
/// @brief 关系表达式指令类定义
///
/// @version 1.0
/// @date 2025-05-07
///

#pragma once

#include <string>
#include "Value.h"
#include "Instruction.h"

///
/// @brief 关系表达式指令类
///
class RelInstruction : public Instruction {
public:
    /// @brief 构造函数
    /// @param _func 所属函数
    /// @param _op 操作符（如 <, <=, >, >=）
    /// @param _srcVal1 左操作数
    /// @param _srcVal2 右操作数
    /// @param _type 结果类型（通常为布尔类型 i1）
    RelInstruction(Function * _func, IRInstOperator _op, Value * _srcVal1, Value * _srcVal2, Type * _type);

    /// @brief 转换成字符串
    /// @param str 转换后的字符串
    void toString(std::string & str) override;
};