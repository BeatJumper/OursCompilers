///
/// @file GetelementptrInstruction.h
/// @brief getelementptr指令，用于计算数组元素地址
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

/// @brief getelementptr指令，用于计算数组元素地址
class GetelementptrInstruction : public Instruction {
protected:
    Value * basePtr;     // 基础指针（数组变量）
    Value * firstIndex;  // 第一个索引（通常是0）
    Value * secondIndex; // 第二个索引（实际数组索引）
    bool inbounds;       // 是否使用inbounds标记

public:
    /// @brief 构造函数
    /// @param _func 所属函数
    /// @param _basePtr 基础指针
    /// @param _index 索引值
    /// @param _inbounds 是否inbounds
    GetelementptrInstruction(Function * _func, Value * _basePtr, Value * _index, bool _inbounds = true);

    /// @brief 构造函数（两个索引版本）
    /// @param _func 所属函数
    /// @param _basePtr 基础指针
    /// @param _firstIndex 第一个索引
    /// @param _secondIndex 第二个索引
    /// @param _inbounds 是否inbounds
    GetelementptrInstruction(Function * _func,
                             Value * _basePtr,
                             Value * _firstIndex,
                             Value * _secondIndex,
                             bool _inbounds = true);

    /// @brief 转换成IR指令字符串
    void toString(std::string & str) override;

    /// @brief 获取基础指针
    Value * getBasePtr() const
    {
        return basePtr;
    }

    /// @brief 获取索引
    Value * getIndex() const
    {
        return firstIndex;
    }
};