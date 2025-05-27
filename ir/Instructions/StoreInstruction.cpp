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
    Value * value = getOperand(0); // 要存储的值
    Value * ptr = getOperand(1);   // 存储的目标地址

    if (!value || !ptr) {
        str = "store error";
        return;
    }

    // 修复：确保指针类型正确显示星号
    std::string valueTypeStr = value->getType()->toString();
    std::string ptrTypeStr = ptr->getType()->toString();

    // 如果指针类型字符串不包含星号，则添加星号
    if (ptrTypeStr.find('*') == std::string::npos) {
        ptrTypeStr += "*";
    }

    // 修复：确保不重复添加星号
    // value的类型应该是基本类型（如i32）
    // ptr的类型应该是指针类型（如i32*）
    str = "store " + valueTypeStr + " " + value->getIRName() + ", " + ptrTypeStr + " " + ptr->getIRName() + ", align " +
          std::to_string(align);
}