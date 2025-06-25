///
/// @file MemcpyInstruction.cpp
/// @brief memcpy指令实现，用于数组初始化等内存拷贝操作
///
/// @author kangyk (2921006018@qq.com)
/// @version 1.0
/// @date 2025-05-25
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-05-25 <td>1.0     <td>kangyk  <td>新建
/// </table>
///

#include "MemcpyInstruction.h"
#include "VoidType.h"

/// @brief 构造函数
/// @param _func 所属函数
/// @param _dest 目标地址
/// @param _src 源地址
/// @param _size 拷贝大小
/// @param _volatile 是否volatile
MemcpyInstruction::MemcpyInstruction(Function * _func, Value * _dest, Value * _src, Value * _size, bool _volatile)
    : Instruction(_func, IRInstOperator::IRINST_OP_MEMCPY, VoidType::getType()), dest(_dest), src(_src), size(_size),
      isVolatile(_volatile)
{
    // 添加操作数
    addOperand(_dest);
    addOperand(_src);
    addOperand(_size);
}

/// @brief 转换成IR指令字符串
void MemcpyInstruction::toString(std::string & str)
{
    // 格式：call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %dest, i8* align 16 %src, i64 20, i1 false)
    str = "call void @llvm.memcpy.p0i8.p0i8.i64(";
    str += "i8* align 16 " + dest->getIRName();
    str += ", i8* align 16 " + src->getIRName();
    str += ", i64 " + size->getIRName();
    str += ", i1 " + std::string(isVolatile ? "true" : "false");
    str += ")";
}