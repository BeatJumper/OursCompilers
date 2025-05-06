///
/// @file BranchInstruction.h
/// @brief 条件跳转指令类定义
///

#pragma once

#include "Instruction.h"
#include "LabelInstruction.h"

///
/// @brief 条件跳转指令
///
class BranchInstruction : public Instruction {
public:
    /// @brief 构造函数
    /// @param _func 所属函数
    /// @param _cond 条件表达式的值
    /// @param _trueLabel 条件为真时跳转的目标标签
    /// @param _falseLabel 条件为假时跳转的目标标签
    BranchInstruction(Function * _func, Value * _cond, LabelInstruction * _trueLabel, LabelInstruction * _falseLabel);

    /// @brief 转换成字符串
    /// @param str 转换后的字符串
    void toString(std::string & str) override;

    /// @brief 获取条件值
    /// @return 条件值
    Value * getCondition() const;

    /// @brief 获取真分支目标标签
    /// @return 真分支目标标签
    LabelInstruction * getTrueLabel() const;

    /// @brief 获取假分支目标标签
    /// @return 假分支目标标签
    LabelInstruction * getFalseLabel() const;

private:
    Value * condition;             ///< 条件表达式的值
    LabelInstruction * trueLabel;  ///< 条件为真时跳转的目标标签
    LabelInstruction * falseLabel; ///< 条件为假时跳转的目标标签
};