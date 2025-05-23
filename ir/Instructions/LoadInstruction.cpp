#include "LoadInstruction.h"

LoadInstruction::LoadInstruction(Function * _func, Value * _result, Value * _ptr, int _align)
    : Instruction(_func, IRInstOperator::IRINST_OP_LOAD, _result->getType()), align(_align)
{
    addOperand(_result);
    addOperand(_ptr);
}

void LoadInstruction::toString(std::string & str)
{
    // Value * result = getOperand(0);
    Value * ptr = getOperand(1);
    str = getIRName() + " = load " + getType()->toString() + ", " + ptr->getType()->toString() + "* " +
          ptr->getIRName() + ", align " + std::to_string(align);
}