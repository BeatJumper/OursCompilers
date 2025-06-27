///
/// @file LabelInstruction.h
/// @brief Label指令
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#pragma once

#include <string>

#include "Instruction.h"

class Function;
class Module;

///
/// @brief Label指令
///
class LabelInstruction : public Instruction {

public:
    ///
    /// @brief 构造函数
    /// @param _func 所属函数
    ///
    explicit LabelInstruction(Function * _func);

    ///
    /// @brief 构造函数（使用Module生成全局唯一标签）
    /// @param _func 所属函数
    /// @param _module 所属模块
    ///
    LabelInstruction(Function * _func, Module * _module);

    ///
    /// @brief 转换成字符串
    /// @param str 返回指令字符串
    ///
    void toString(std::string & str) override;
};
