///
/// @file ZextInstruction.cpp
/// @brief ZEXT（零扩展）指令实现
///
#include "ZextInstruction.h"

/// @brief 构造函数
ZextInstruction::ZextInstruction(Function * _func, Value * _srcVal, Type * _targetType)
    : Instruction(_func, IRInstOperator::IRINST_OP_ZEXT, _targetType)
{
    addOperand(_srcVal);
}

/// @brief 转换成字符串
void ZextInstruction::toString(std::string & str)
{
    Value * src = getOperand(0);

    str =
        getIRName() + " = zext " + src->getType()->toString() + " " + src->getIRName() + " to " + getType()->toString();
}