///
/// @file XorInstruction.cpp
/// @brief XOR（异或）指令实现
///
#include "XorInstruction.h"

/// @brief 构造函数
XorInstruction::XorInstruction(Function * _func, Value * _srcVal1, Value * _srcVal2, Type * _type)
    : Instruction(_func, IRInstOperator::IRINST_OP_XOR_I, _type)
{
    addOperand(_srcVal1);
    addOperand(_srcVal2);
}

/// @brief 转换成字符串
void XorInstruction::toString(std::string & str)
{
    Value * left = getOperand(0);
    Value * right = getOperand(1);

    str = getIRName() + " = xor " + left->getType()->toString() + " " + left->getIRName() + ", " + right->getIRName();
}