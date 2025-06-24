#include "StackLdrInstruction.h"
#include "VoidType.h"

StackLdrInstruction::StackLdrInstruction(Function * _func, Value * _value, Type * _type)
    : Instruction(_func, IRInstOperator::IRINST_OP_STACKLDR, _type)
{
    this->val = _value;
}

void StackLdrInstruction::toString(std::string & str)
{
    str = "";
}