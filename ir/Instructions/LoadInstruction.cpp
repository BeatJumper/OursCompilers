#include "LoadInstruction.h"
#include "PointerType.h"
#include "IntegerType.h"

LoadInstruction::LoadInstruction(Function * _func, Value * _result, Value * _ptr, int _align)
    : Instruction(_func, IRInstOperator::IRINST_OP_LOAD, _result->getType()), align(_align)
{
    addOperand(_result); // 只添加要加载的地址作为操作数

    // 设置正确的结果类型：如果是指针类型，则结果是指针指向的类型
    if (_ptr->getType()->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(_ptr->getType());
        this->type = const_cast<Type *>(ptrType->getPointeeType());
    } else {
        // 对于非指针类型（如变量），结果类型与变量类型相同
        this->type = _ptr->getType();
    }
}

void LoadInstruction::toString(std::string & str)
{
    Value * ptr = getOperand(0); // 第一个操作数就是地址
    if (!ptr) {
        str = getIRName() + " = load error";
        return;
    }

    std::string resultTypeStr = getType()->toString();
    std::string ptrTypeStr = ptr->getType()->toString();

    // 确保指针类型正确显示星号
    if (ptrTypeStr.find('*') == std::string::npos) {
        ptrTypeStr += "*";
    }

    str = getIRName() + " = load " + resultTypeStr + ", " + ptrTypeStr + " " + ptr->getIRName() + ", align " +
          std::to_string(align);
}