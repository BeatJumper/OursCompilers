///
/// @file MemcpyInstruction.h
/// @brief Memcpy指令
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

/// @brief memcpy指令，用于数组初始化等内存拷贝操作
class MemcpyInstruction : public Instruction {
protected:
    Value * dest;    // 目标地址
    Value * src;     // 源地址
    Value * size;    // 拷贝大小
    bool isVolatile; // 是否volatile

public:
    /// @brief 构造函数
    MemcpyInstruction(Function * _func, Value * _dest, Value * _src, Value * _size, bool _volatile = false);

    /// @brief 转换成IR指令字符串
    void toString(std::string & str) override;
};