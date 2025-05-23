///
/// @file ExitInstruction.cpp
/// @brief 函数出口或返回指令
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
#include "VoidType.h"

#include "ExitInstruction.h"

/// 要不要改成LLVM IR呢，如果改成LLVM格式的话，那么源文件的名字exit开头就不合适了
/// 那么就改成LLVM IR的格式吧，文件名字改成ret好一些，但是这样的话这个文件有很多依赖项
/// 需要改动，还是算了，先不改了，就用这个exit的名字来承担return的功能吧

/// @brief 没有返回值的return语句指令
/// @param _result 返回结果值
ExitInstruction::ExitInstruction(Function * _func)
    : Instruction(_func, IRInstOperator::IRINST_OP_RET, VoidType::getType())
{
    // 没有返回值的return语句
    // 这里不需要添加操作数
}

/// @brief return语句指令
/// @param _result 返回结果值
ExitInstruction::ExitInstruction(Function * _func, Value * _result)
    : Instruction(_func, IRInstOperator::IRINST_OP_RET, VoidType::getType())
{
    if (_result != nullptr) {
        addOperand(_result);
    }
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void ExitInstruction::toString(std::string & str)
{
    if (getOperandsNum() == 0) {
        str = "ret void";
    } else {
        Value * retValue = getOperand(0);
        str = "ret " + retValue->getType()->toString() + " " + retValue->getIRName();
    }
}
