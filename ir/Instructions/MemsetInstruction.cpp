///
/// @file MemsetInstruction.cpp
/// @brief memset指令实现，用于数组零初始化等内存设置操作
///
/// @author kyk
/// @version 1.0
/// @date 2024-12-26
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-12-26 <td>1.0     <td>kyk     <td>新建
/// </table>
///

#include "MemsetInstruction.h"
#include "VoidType.h"

/// @brief 构造函数
/// @param _func 所属函数
/// @param _dest 目标地址
/// @param _value 设置的值
/// @param _size 设置大小
/// @param _volatile 是否volatile
MemsetInstruction::MemsetInstruction(Function * _func, Value * _dest, Value * _value, Value * _size, bool _volatile)
    : Instruction(_func, IRInstOperator::IRINST_OP_MEMSET, VoidType::getType()), dest(_dest), value(_value),
      size(_size), isVolatile(_volatile)
{
    // 添加操作数
    addOperand(_dest);
    addOperand(_value);
    addOperand(_size);
}

/// @brief 转换成IR指令字符串
void MemsetInstruction::toString(std::string & str)
{
    // 格式：call void @llvm.memset.p0i8.i64(i8* align 16 %dest, i8 0, i64 32, i1 false)
    str = "call void @llvm.memset.p0i8.i64(";
    str += "i8* align 16 " + dest->getIRName();
    str += ", i8 " + value->getIRName();
    str += ", i64 " + size->getIRName();
    str += ", i1 " + std::string(isVolatile ? "true" : "false");
    str += ")";
}
