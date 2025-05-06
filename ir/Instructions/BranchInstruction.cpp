///
/// @file BranchInstruction.cpp
/// @brief 条件跳转指令类实现
///

#include "BranchInstruction.h"
#include "VoidType.h"

///
/// @brief 构造函数
/// @param _func 所属函数
/// @param _cond 条件表达式的值
/// @param _trueLabel 条件为真时跳转的目标标签
/// @param _falseLabel 条件为假时跳转的目标标签
BranchInstruction::BranchInstruction(Function * _func,
                                     Value * _cond,
                                     LabelInstruction * _trueLabel,
                                     LabelInstruction * _falseLabel)
    : Instruction(_func, IRInstOperator::IRINST_OP_BRANCH, VoidType::getType()), condition(_cond),
      trueLabel(_trueLabel), falseLabel(_falseLabel)
{}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void BranchInstruction::toString(std::string & str)
{
    str = "bc " + condition->getIRName() + ", label " + trueLabel->getIRName() + ", label " + falseLabel->getIRName();
}

/// @brief 获取条件值
/// @return 条件值
Value * BranchInstruction::getCondition() const
{
    return condition;
}

/// @brief 获取真分支目标标签
/// @return 真分支目标标签
LabelInstruction * BranchInstruction::getTrueLabel() const
{
    return trueLabel;
}

/// @brief 获取假分支目标标签
/// @return 假分支目标标签
LabelInstruction * BranchInstruction::getFalseLabel() const
{
    return falseLabel;
}