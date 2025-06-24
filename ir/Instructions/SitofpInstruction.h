///
/// @file SitofpInstruction.h
/// @brief sitofp指令类，用于有符号整数转浮点数
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

#pragma once

#include "Instruction.h"

/// @brief sitofp指令，用于有符号整数转浮点数
class SitofpInstruction : public Instruction {
protected:
    Value * srcValue;  // 源值
    Type * targetType; // 目标类型

public:
    /// @brief 构造函数
    SitofpInstruction(Function * _func, Value * _srcValue, Type * _targetType);

    /// @brief 转换成IR指令字符串
    void toString(std::string & str) override;
};
