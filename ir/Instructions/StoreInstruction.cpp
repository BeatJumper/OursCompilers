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

    // 获取类型字符串
    std::string valueTypeStr = value->getType()->toString();
    std::string ptrTypeStr = ptr->getType()->toString();

    // LLVM IR store指令格式：store <ty> <value>, <ty>* <pointer>, align <alignment>
    // 其中<ty>*表示指向<ty>类型的指针
    // 例如：store i32 %0, i32* %1, align 4
    //      store i32* %0, i32** %1, align 8
    str = "store " + valueTypeStr + " " + value->getIRName() + ", " + ptrTypeStr + " " + ptr->getIRName() + ", align " +
          std::to_string(align);
}