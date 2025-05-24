///
/// @file BinaryInstruction.cpp
/// @brief 二元操作指令
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#include "BinaryInstruction.h"

/// @brief 构造函数
/// @param _op 操作符
/// @param _result 结果操作数
/// @param _srcVal1 源操作数1
/// @param _srcVal2 源操作数2
BinaryInstruction::BinaryInstruction(Function * _func,
                                     IRInstOperator _op,
                                     Value * _srcVal1,
                                     Value * _srcVal2,
                                     Type * _type)
    : Instruction(_func, _op, _type)
{
    addOperand(_srcVal1);
    addOperand(_srcVal2);
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void BinaryInstruction::toString(std::string & str)
{
    std::string opStr;

    switch (getOp()) {
        case IRInstOperator::IRINST_OP_ADD_I:
            opStr = "add nsw";
            break;
        case IRInstOperator::IRINST_OP_SUB_I:
            opStr = "sub nsw";
            break;
        case IRInstOperator::IRINST_OP_MUL_I:
            opStr = "mul nsw";
            break;
        case IRInstOperator::IRINST_OP_DIV_I:
            opStr = "sdiv";
            break;
        case IRInstOperator::IRINST_OP_MOD_I: // 新增
            opStr = "srem";
            break;
        case IRInstOperator::IRINST_OP_XOR_I: // 新增
            opStr = "xor";
            break;
        default:
            opStr = "unknown";
            break;
    }

    Value * left = getOperand(0);
    Value * right = getOperand(1);

    if (getOp() == IRInstOperator::IRINST_OP_XOR_I) {
        // XOR指令不需要nsw标志
        str = getIRName() + " = " + opStr + " " + left->getType()->toString() + " " + left->getIRName() + ", " +
              right->getIRName();
    } else {
        str = getIRName() + " = " + opStr + " " + left->getType()->toString() + " " + left->getIRName() + ", " +
              right->getIRName();
    }
}