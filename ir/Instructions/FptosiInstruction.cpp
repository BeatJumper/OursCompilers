///
/// @file FptosiInstruction.cpp
/// @brief fptosi指令实现，用于浮点数转有符号整数
///
/// @author kangyk (2921006018@qq.com)
/// @version 1.0
/// @date 2025-01-15
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-01-15 <td>1.0     <td>kangyk  <td>新建
/// </table>
///

#include "FptosiInstruction.h"

/// @brief 构造函数
/// @param _func 所属函数
/// @param _srcValue 源值
/// @param _targetType 目标类型
FptosiInstruction::FptosiInstruction(Function * _func, Value * _srcValue, Type * _targetType)
    : Instruction(_func, IRInstOperator::IRINST_OP_FPTOSI, _targetType), srcValue(_srcValue), targetType(_targetType)
{
    // 添加操作数
    addOperand(_srcValue);
}

/// @brief 转换成IR指令字符串
void FptosiInstruction::toString(std::string & str)
{
    // 格式：%result = fptosi float %value to i32
    str = getIRName() + " = fptosi ";
    
    Type * srcType = srcValue->getType();
    str += srcType->toString() + " " + srcValue->getIRName();
    str += " to " + targetType->toString();
}
