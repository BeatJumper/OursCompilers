///
/// @file Instruction.cpp
/// @brief IR指令类实现
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// </table>
///
#include <string>

#include "Instruction.h"
#include "Function.h"
#include "AllocaInstruction.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
#include "MoveInstruction.h"
#include "FuncCallInstruction.h"
#include "GetelementptrInstruction.h"
#include "PlatformArm64.h"

/// @brief 构造函数
/// @param op
/// @param result
/// @param srcVal1
/// @param srcVal2
Instruction::Instruction(Function * _func, IRInstOperator _op, Type * _type) : User(_type), op(_op), func(_func)
{}

/// @brief 获取指令操作码
/// @return 指令操作码
IRInstOperator Instruction::getOp()
{
    return op;
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void Instruction::toString(std::string & str)
{
    // 未知指令
    str = "Unkown IR Instruction";
}

/// @brief 是否是Dead指令
bool Instruction::isDead()
{
    return dead;
}

/// @brief 设置指令是否是Dead指令
/// @param _dead 是否是Dead指令，true：Dead, false: 非Dead
void Instruction::setDead(bool _dead)
{
    dead = _dead;
}

///
/// @brief 获取当前指令所在函数
/// @return Function* 函数对象
///
Function * Instruction::getFunction()
{
    return func;
}

///
/// @brief 检查指令是否有值
/// @return true
/// @return false
///
bool Instruction::hasResultValue()
{
    return !type->isVoidType();
}

void Instruction::transfer()
{
    if (Instanceof(inst, AllocaInstruction *, this)) {
        // Alloc指令没有直接数据流，所以不做任何事
    } else if (Instanceof(inst, StoreInstruction *, this)) {
        use_set.insert(inst->getOperand(0));
    } else if (Instanceof(inst, LoadInstruction *, this)) {
        def_set.insert(inst);
    } else if (Instanceof(inst, MoveInstruction *, this)) {
        def_set.insert(inst->getOperand(0));
        Value * source = inst->getOperand(1);
        if (Instanceof(constvar, Constant *, source)) {
            // 什么都不做
        } else {
            use_set.insert(source);
        }
    } else if (Instanceof(inst, FuncCallInstruction *, this)) {
        // 在DEF集上，添加18个DEF，表示函数调用使得寄存器W0~W17都可能遭到修改
        for (int index = 0; index < PlatformArm64::CallerSaveRegNum; index++) {
            Value * val = inst->getOperand(index);
            val->setRegId(index);
            def_set.insert(val);
        }

        // 接下来是USE集的添加

        // 前8个数的临时变量被直接用到
        for (int index = 0; index < 8 && index < inst->getOperandsNum(); index++) {
            use_set.insert(inst->getOperand(index));
        }
        // 后8个数是仅仅在内存里的，不占寄存器，所以就不进USE了。
    } else if (Instanceof(inst, Instruction *, this)) {
        // printf("其它指令\n");
        if (inst->hasResultValue()) {
            def_set.insert(inst);
        }

        for (auto usee: inst->getOperandsValue()) {
            // 除了store指令以外的立即数都不需要寄存器
            if (Instanceof(constusee, Constant *, usee) == nullptr && def_set.count(usee) == 0) {
                // use集中不能包含刚刚def的元素
                use_set.insert(usee);
            }
        }
    }

    // 提前初始化liveIN
    liveIN = use_set;
}