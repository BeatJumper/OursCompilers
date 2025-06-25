///
/// @file SitofpInstruction.cpp
/// @brief sitofp指令实现，用于有符号整数转浮点数
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

#include "SitofpInstruction.h"

/// @brief 构造函数
/// @param _func 所属函数
/// @param _srcValue 源值
/// @param _targetType 目标类型
SitofpInstruction::SitofpInstruction(Function * _func, Value * _srcValue, Type * _targetType)
    : Instruction(_func, IRInstOperator::IRINST_OP_SITOFP, _targetType), srcValue(_srcValue), targetType(_targetType)
{
    // 添加操作数
    addOperand(_srcValue);
}

/// @brief 转换成IR指令字符串
void SitofpInstruction::toString(std::string & str)
{
    // 格式：%result = sitofp i32 %value to float
    str = getIRName() + " = sitofp ";
    
    Type * srcType = srcValue->getType();
    str += srcType->toString() + " " + srcValue->getIRName();
    str += " to " + targetType->toString();
}
