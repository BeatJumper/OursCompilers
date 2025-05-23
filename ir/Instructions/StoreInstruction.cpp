#include "StoreInstruction.h"
#include "VoidType.h"

StoreInstruction::StoreInstruction(Function * _func, Value * _value, Value * _ptr, int _align)
    : Instruction(_func, IRInstOperator::IRINST_OP_STORE, VoidType::getType()), align(_align)
{
    addOperand(_value);
    addOperand(_ptr);
}

void StoreInstruction::toString(std::string & str)
{
    Value * value = getOperand(0);
    Value * ptr = getOperand(1);
    str = "store " + value->getType()->toString() + " " + value->getIRName() + ", " + ptr->getType()->toString() +
          "* " + ptr->getIRName() + ", align " + std::to_string(align);
}