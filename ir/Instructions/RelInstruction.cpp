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

    // 检查操作数类型，决定使用icmp还是fcmp
    bool isFloatComparison = left->getType()->isFloatType() || right->getType()->isFloatType();

    std::string condStr;
    std::string cmpInst = isFloatComparison ? "fcmp" : "icmp";

    if (isFloatComparison) {
        // 浮点数比较使用fcmp指令
        switch (op) {
            case IRInstOperator::IRINST_OP_EQ:
                condStr = "oeq"; // ordered equal
                break;
            case IRInstOperator::IRINST_OP_NE:
                condStr = "one"; // ordered not equal
                break;
            case IRInstOperator::IRINST_OP_LT:
                condStr = "olt"; // ordered less than
                break;
            case IRInstOperator::IRINST_OP_LE:
                condStr = "ole"; // ordered less than or equal
                break;
            case IRInstOperator::IRINST_OP_GT:
                condStr = "ogt"; // ordered greater than
                break;
            case IRInstOperator::IRINST_OP_GE:
                condStr = "oge"; // ordered greater than or equal
                break;
            default:
                condStr = "oeq";
                break;
        }
    } else {
        // 整数比较使用icmp指令
        switch (op) {
            case IRInstOperator::IRINST_OP_EQ:
                condStr = "eq";
                break;
            case IRInstOperator::IRINST_OP_NE:
                condStr = "ne";
                break;
            case IRInstOperator::IRINST_OP_LT:
                condStr = "slt"; // signed less than
                break;
            case IRInstOperator::IRINST_OP_LE:
                condStr = "sle"; // signed less than or equal
                break;
            case IRInstOperator::IRINST_OP_GT:
                condStr = "sgt"; // signed greater than
                break;
            case IRInstOperator::IRINST_OP_GE:
                condStr = "sge"; // signed greater than or equal
                break;
            default:
                condStr = "eq";
                break;
        }
    }

    // 确保两个操作数使用相同的类型
    std::string leftType = left->getType()->toString();

    // 如果是浮点数比较，确保两个操作数都使用浮点数类型
    if (isFloatComparison) {
        str = getIRName() + " = " + cmpInst + " " + condStr + " float " + left->getIRName() + ", " + right->getIRName();
    } else {
        str = getIRName() + " = " + cmpInst + " " + condStr + " " + leftType + " " + left->getIRName() + ", " +
              right->getIRName();
    }
}