///
/// @file LabelInstruction.cpp
/// @brief Label指令
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
#include "VoidType.h"
#include "Module.h"

#include "LabelInstruction.h"

///
/// @brief 构造函数
/// @param _func 所属函数
///
LabelInstruction::LabelInstruction(Function * _func)
    : Instruction(_func, IRInstOperator::IRINST_OP_LABEL, VoidType::getType())
{}

///
/// @brief 构造函数（使用Module生成全局唯一标签）
/// @param _func 所属函数
/// @param _module 所属模块
///
LabelInstruction::LabelInstruction(Function * _func, Module * _module)
    : Instruction(_func, IRInstOperator::IRINST_OP_LABEL, VoidType::getType())
{
    // 使用全局标签计数器生成唯一标签名
    int32_t labelId = _module->getNextLabelId();

    // 生成带函数名前缀的标签名：函数名_数字
    std::string funcName = _func->getName();
    std::string labelName = funcName + "_" + std::to_string(labelId);
    setIRName(labelName);
}

/// @brief 转换成字符串
/// @param str 返回指令字符串
void LabelInstruction::toString(std::string & str)
{
    // str = IRName + ":";
    // 改成这样试试
    str = getIRName() + ":";
}
