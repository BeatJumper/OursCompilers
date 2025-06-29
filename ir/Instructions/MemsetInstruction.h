///
/// @file MemsetInstruction.h
/// @brief Memset指令
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

#pragma once

#include "Instruction.h"

/// @brief memset指令，用于数组零初始化等内存设置操作
class MemsetInstruction : public Instruction {
protected:
    Value * dest;    // 目标地址
    Value * value;   // 设置的值（通常是0）
    Value * size;    // 设置大小
    bool isVolatile; // 是否volatile

public:
    /// @brief 构造函数
    MemsetInstruction(Function * _func, Value * _dest, Value * _value, Value * _size, bool _volatile = false);

    /// @brief 转换成IR指令字符串
    void toString(std::string & str) override;
};
