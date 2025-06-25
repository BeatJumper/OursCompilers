///
/// @file BitcastInstruction.h
/// @brief bitcast指令，用于指针类型转换
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

#pragma once

#include "Instruction.h"

/// @brief bitcast指令，用于指针类型转换
class BitcastInstruction : public Instruction {
protected:
    Value * srcValue;  // 源值
    Type * targetType; // 目标类型

public:
    /// @brief 构造函数
    BitcastInstruction(Function * _func, Value * _srcValue, Type * _targetType);

    /// @brief 转换成IR指令字符串
    void toString(std::string & str) override;
};