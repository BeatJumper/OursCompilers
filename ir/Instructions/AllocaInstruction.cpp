///
/// @file ArgInstruction.cpp
/// @brief 分配内存指令
/// @author kangyk (2921006018@qq.com)
/// @version 1.0
/// @date 2025-05-24
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-05-24 <td>1.0     <td>kangyk  <td>新建
/// </table>
///

#include "AllocaInstruction.h"
#include "VoidType.h"

AllocaInstruction::AllocaInstruction(Function * _func, Value * _result, Type * _type, int _align)
    : Instruction(_func, IRInstOperator::IRINST_OP_ALLOCA, _type), align(_align)
{
    addOperand(_result);
    // is_leaked = true;
    // ptr = _result;
}

void AllocaInstruction::toString(std::string & str)
{
    Value * result = getOperand(0);
    // Value * result = ptr;
    str = result->getIRName() + " = alloca " + getType()->toString() + ", align " + std::to_string(align);

    // 添加分号和注释，显示变量名字
    std::string realName = result->getName();
    if (!realName.empty()) {
        str += " ; " + realName;
    } else {
        str += " ; temp_var"; // 如果没有名字，标记为临时变量
    }
}

/// @brief 获取size
int AllocaInstruction::getAligned()
{
    return align;
}
