///
/// @file SextInstruction.cpp
/// @brief sext指令实现，用于符号扩展（i32 -> i64）
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

#include "SextInstruction.h"
#include "PointerType.h"

/// @brief 构造函数
/// @param _func 所属函数
/// @param _srcValue 源值
/// @param _targetType 目标类型
SextInstruction::SextInstruction(Function * _func, Value * _srcValue, Type * _targetType)
    : Instruction(_func, IRInstOperator::IRINST_OP_SEXT, _targetType), srcValue(_srcValue), targetType(_targetType)
{
    // 添加操作数
    addOperand(_srcValue);
}

/// @brief 转换成IR指令字符串
void SextInstruction::toString(std::string & str)
{
    // 格式：%result = sext i32 %value to i64
    str = getIRName() + " = sext ";

    // 修复：确保获取的是值的类型，而不是指针类型
    Type * srcType = srcValue->getType();
    if (srcType->isPointerType()) {
        // 如果源值是指针类型，获取指向的类型
        const PointerType * ptrType = static_cast<const PointerType *>(srcType);
        srcType = const_cast<Type *>(ptrType->getPointeeType());
    }

    str += srcType->toString() + " " + srcValue->getIRName();
    str += " to " + targetType->toString();
}