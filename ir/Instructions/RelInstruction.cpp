///
/// @file RelInstruction.cpp
/// @brief 关系表达式指令类实现
///

#include "RelInstruction.h"
#include "Instruction.h"

///
/// @brief 构造函数
/// @param _func 所属函数
/// @param _op 操作符（如 <, <=, >, >=）
/// @param _srcVal1 左操作数
/// @param _srcVal2 右操作数
/// @param _type 结果类型（通常为布尔类型 i1）
RelInstruction::RelInstruction(Function * _func, IRInstOperator _op, Value * _srcVal1, Value * _srcVal2, Type * _type)
    : Instruction(_func, _op, _type)
{
    addOperand(_srcVal1);
    addOperand(_srcVal2);
}

///
/// @brief 转换成字符串
/// @param str 转换后的字符串
void RelInstruction::toString(std::string & str)
{
    Value * left = getOperand(0);
    Value * right = getOperand(1);

    switch (op) {
        case IRInstOperator::IRINST_OP_LT:
            str = getIRName() + " = cmp lt " + left->getIRName() + ", " + right->getIRName();
            break;
        case IRInstOperator::IRINST_OP_LE:
            str = getIRName() + " = cmp le " + left->getIRName() + ", " + right->getIRName();
            break;
        case IRInstOperator::IRINST_OP_GT:
            str = getIRName() + " = cmp gt " + left->getIRName() + ", " + right->getIRName();
            break;
        case IRInstOperator::IRINST_OP_GE:
            str = getIRName() + " = cmp ge " + left->getIRName() + ", " + right->getIRName();
            break;
        case IRInstOperator::IRINST_OP_EQ:
            str = getIRName() + " = cmp eq " + left->getIRName() + ", " + right->getIRName();
            break;
        case IRInstOperator::IRINST_OP_NE:
            str = getIRName() + " = cmp ne " + left->getIRName() + ", " + right->getIRName();
            break;
        default:
            str = "unknown rel instruction";
            break;
    }
}