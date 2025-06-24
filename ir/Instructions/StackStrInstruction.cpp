#include "StackStrInstruction.h"
#include "VoidType.h"

StackStrInstruction::StackStrInstruction(Function * _func, Value * _value)
    : Instruction(_func, IRInstOperator::IRINST_OP_STACKSTR, VoidType::getType())
{
    addOperand(_value);
}

void StackStrInstruction::toString(std::string & str)
{
    Value * value = getOperand(0);
    str = "";
}