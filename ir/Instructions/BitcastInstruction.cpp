///
/// @file BitcastInstruction.cpp
/// @brief BitcastInstruction类的实现文件
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

#include "BitcastInstruction.h"

/// @brief 构造函数
/// @param _func 所属函数
/// @param _srcValue 源值
/// @param _targetType 目标类型
BitcastInstruction::BitcastInstruction(Function * _func, Value * _srcValue, Type * _targetType)
    : Instruction(_func, IRInstOperator::IRINST_OP_BITCAST, _targetType), srcValue(_srcValue), targetType(_targetType)
{
    // 添加操作数
    addOperand(_srcValue);
}

/// @brief 转换成IR指令字符串
void BitcastInstruction::toString(std::string & str)
{
    str = getIRName() + " = bitcast ";

    // 修复：确保源类型正确显示为指针类型
    Type * srcType = srcValue->getType();
    if (srcType->isArrayType()) {
        str += srcType->toString() + "* " + srcValue->getIRName();
    } else {
        str += srcType->toString() + " " + srcValue->getIRName();
    }

    str += " to " + targetType->toString();
}