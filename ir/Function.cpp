///
/// @file Function.cpp
/// @brief 函数实现
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

#include <cstdlib>
#include <string>
#include <set>

#include "IRConstant.h"
#include "Function.h"
#include "Module.h"
#include "Liveness.h"
#include "CFG.h"

/// @brief 指定函数名字、函数类型的构造函数
/// @param _name 函数名称
/// @param _type 函数类型
/// @param _builtin 是否是内置函数
Function::Function(std::string _name, FunctionType * _type, bool _builtin)
    : GlobalValue(_type, _name), builtIn(_builtin)
{
    returnType = _type->getReturnType();

    // 设置对齐大小
    setAlignment(2);
}

///
/// @brief 析构函数
/// @brief 释放函数占用的内存和IR指令代码
/// @brief 注意：IR指令代码并未释放，需要手动释放
Function::~Function()
{
    Delete();
}

/// @brief 获取函数返回类型
/// @return 返回类型
Type * Function::getReturnType()
{
    return returnType;
}

/// @brief 获取函数的形参列表
/// @return 形参列表
std::vector<FormalParam *> & Function::getParams()
{
    return params;
}

/// @brief 获取函数内的IR指令代码
/// @return IR指令代码
InterCode & Function::getInterCode()
{
    return code;
}

/// @brief 判断该函数是否是内置函数
/// @return true: 内置函数，false：用户自定义
bool Function::isBuiltin()
{
    return builtIn;
}

/// @brief 函数指令信息输出
/// @param str 函数指令
void Function::toString(std::string & str)
{
    if (builtIn) {
        // 内置函数则什么都不输出
        return;
    }

    // 输出函数头
    str = "define dso_local " + getReturnType()->toString() + " " + getIRName() + "(";

    bool firstParam = false;
    for (auto & param: params) {

        if (!firstParam) {
            firstParam = true;
        } else {
            str += ", ";
        }

        std::string param_str = param->getType()->toString() + " noundef " + param->getIRName();

        str += param_str;
    }

    str += ") #0 {\n";

    // BeatJumper：我猜这里得注释掉，实现指令类解耦，否则是在不宜管理

    /*
    // 输出局部变量的名字与IR名字
    for (auto & var: this->varsVector) {

        // 局部变量和临时变量需要输出declare语句
        // TODO LLVM IR
        str += "\tdeclare " + var->getType()->toString() + " " + var->getIRName();

        std::string extraStr;
        std::string realName = var->getName();
        if (!realName.empty()) {
            str += " ; " + std::to_string(var->getScopeLevel()) + ":" + realName;
        }

        str += "\n";
    }
    */

    // BeatJumper：我猜这里得注释掉，实现指令类解耦，否则是在不宜管理

    /*
    // 输出临时变量的declare形式
    // 遍历所有的线性IR指令，文本输出
    for (auto & inst: code.getInsts()) {

        if (inst->hasResultValue()) {

            // 局部变量和临时变量需要输出declare语句
            str += "\tdeclare " + inst->getType()->toString() + " " + inst->getIRName() + "\n";
        }
    }
    */

    // 遍历所有的线性IR指令，文本输出
    for (auto & inst: code.getInsts()) {

        std::string instStr;
        inst->toString(instStr);

        if (!instStr.empty()) {

            // Label指令不加Tab键
            if (inst->getOp() == IRInstOperator::IRINST_OP_LABEL) {
                str += instStr + "\n";
            } else {
                str += "\t" + instStr + "\n";
            }
        }
    }

    // 输出函数尾部
    str += "}\n";
}

/// @brief 设置函数出口指令
/// @param inst 出口Label指令
void Function::setExitLabel(Instruction * inst)
{
    exitLabel = inst;
}

/// @brief 获取函数出口指令
/// @return 出口Label指令
Instruction * Function::getExitLabel()
{
    return exitLabel;
}

/// @brief 设置函数返回值变量
/// @param val 返回值变量，要求必须是局部变量，不能是临时变量
void Function::setReturnValue(LocalVariable * val)
{
    returnValue = val;
}

/// @brief 获取函数返回值变量
/// @return 返回值变量
LocalVariable * Function::getReturnValue()
{
    return returnValue;
}

/// @brief 获取最大栈帧深度
/// @return 栈帧深度
int Function::getMaxDep()
{
    return maxDepth;
}

/// @brief 设置最大栈帧深度
/// @param dep 栈帧深度
void Function::setMaxDep(int dep)
{
    maxDepth = dep;

    // 设置函数栈帧被重定位标记，用于生成不同的栈帧保护代码
    relocated = true;
}

/// @brief 获取本函数需要保护的寄存器
/// @return 要保护的寄存器
std::vector<int32_t> & Function::getProtectedReg()
{
    return protectedRegs;
}

/// @brief 获取本函数需要保护的寄存器字符串
/// @return 要保护的寄存器
std::string & Function::getProtectedRegStr()
{
    return protectedRegStr;
}

std::vector<LocalVariable *> & Function::get_localspace_for_protected()
{
    return localspace_for_protected;
}

std::vector<Value *> & Function::get_regvalue_for_protected()
{
    return regvalue_for_protected;
}

/// @brief 获取函数调用参数个数的最大值
/// @return 函数调用参数个数的最大值
int Function::getMaxFuncCallArgCnt()
{
    return maxFuncCallArgCnt;
}

/// @brief 设置函数调用参数个数的最大值
/// @param count 函数调用参数个数的最大值
void Function::setMaxFuncCallArgCnt(int count)
{
    maxFuncCallArgCnt = count;
}

/// @brief 函数内是否存在函数调用
/// @return 是否存在函调用
bool Function::getExistFuncCall()
{
    return funcCallExist;
}

/// @brief 设置函数是否存在函数调用
/// @param exist true: 存在 false: 不存在
void Function::setExistFuncCall(bool exist)
{
    funcCallExist = exist;
}

/// @brief 新建变量型Value。先检查是否存在，不存在则创建，否则失败
/// @param name 变量ID
/// @param type 变量类型
/// @param scope_level 局部变量的作用域层级
LocalVariable * Function::newLocalVarValue(Type * type, std::string name, int32_t scope_level)
{
    // 创建变量并加入符号表
    LocalVariable * varValue = new LocalVariable(type, name, scope_level);

    // varsVector表中可能存在变量重名的信息
    varsVector.push_back(varValue);

    return varValue;
}

/// @brief 新建一个内存型的Value，并加入到符号表，用于后续释放空间
/// \param type 变量类型
/// \return 临时变量Value
MemVariable * Function::newMemVariable(Type * type)
{
    // 肯定唯一存在，直接插入即可
    MemVariable * memValue = new MemVariable(type);

    memVector.push_back(memValue);

    return memValue;
}

/// @brief 清理函数内申请的资源
void Function::Delete()
{
    // 清理IR指令
    code.Delete();

    // 清理Value
    for (auto & var: varsVector) {
        delete var;
    }

    varsVector.clear();
}

///
/// @brief 函数内的Value重命名
/// @param module 模块指针，用于获取全局计数器
///
void Function::renameIR()
{
    // 内置函数忽略
    if (isBuiltin()) {
        return;
    }

    // printf("==== Starting renameIR for function %s ====\n", this->name.c_str());

    // 每个函数维护两个独立的计数器
    int32_t variableCounter = 0; // 变量计数器：参数、局部变量、临时值
    int32_t labelCounter = 0;    // 标签计数器：标签编号

    // 1. 形式参数重命名 - 按照LLVM IR规范，函数参数必须从%0开始连续编号
    for (auto & param: this->params) {
        param->setIRName(IR_TEMP_VARNAME_PREFIX + std::to_string(variableCounter));
        variableCounter++;
        // printf("Renamed param: %s -> %s\n", oldName.empty() ? "(empty)" : oldName.c_str(),
        // param->getIRName().c_str());
    }

    // 2. 按照alloca指令在IR中的出现顺序重命名局部变量，确保变量名与alloca指令顺序一致
    // 3. 同时处理标签和临时值重命名
    for (auto inst: this->getInterCode().getInsts()) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_LABEL) {
            // 标签重命名，格式：函数名_Lx
            std::string funcName = this->getName();
            std::string labelName = funcName + "_L" + std::to_string(labelCounter);
            inst->setIRName(labelName);
            labelCounter++;
        } else if (inst->getOp() == IRInstOperator::IRINST_OP_ALLOCA) {
            // alloca指令：重命名其结果变量（按照IR中的出现顺序）
            Value * allocaResult = inst->getOperand(0); // alloca的结果是第一个操作数
            if (allocaResult) {
                allocaResult->setIRName(IR_LOCAL_VARNAME_PREFIX + std::to_string(variableCounter));
                variableCounter++;
                /*printf("Renamed alloca result var %s: -> %s\n",
                       allocaResult->getName().c_str(),
                       allocaResult->getIRName().c_str());*/
            }
        } else if (inst->hasResultValue()) {
            // 其他有结果值的指令：临时值重命名（加%前缀）
            inst->setIRName(IR_TEMP_VARNAME_PREFIX + std::to_string(variableCounter));
            variableCounter++;
        }
    }
    for (auto inst: this->getInterCode().getInsts()) {
        for (Value * val: inst->get_def_set()) {
            if (val->getIRName().empty()) {
                val->setIRName(IR_TEMP_VARNAME_PREFIX + std::to_string(variableCounter));
                variableCounter++;
            }
        }
        for (Value * val: inst->get_use_set()) {
            if (val->getIRName().empty()) {
                val->setIRName(IR_TEMP_VARNAME_PREFIX + std::to_string(variableCounter));
                variableCounter++;
            }
        }
    }
    // printf("==== Finished renameIR for function %s ====\n", this->name.c_str());
}

///
/// @brief 获取统计的ARG指令的个数
/// @return int32_t 个数
///
int32_t Function::getRealArgcount()
{
    return this->realArgCount;
}

///
/// @brief 用于统计ARG指令个数的自增函数，个数加1
///
void Function::realArgCountInc()
{
    this->realArgCount++;
}

///
/// @brief 用于统计ARG指令个数的清零
///
void Function::realArgCountReset()
{
    this->realArgCount = 0;
}

///
/// @brief 用于添加基本块
///
void Function::addBasicBlock(InterCode * BasicBlock)
{
    this->BasicBlocks.push_back(BasicBlock);
}

std::vector<InterCode *> & Function::getBasicBlocks()
{
    return BasicBlocks;
}

void Function::clearBasicBlocks()
{
    BasicBlocks.clear();
}

///
/// @brief 获取下一个栈偏移量
///
int64_t Function::getNextStackOffset()
{
    return nextStackOffset;
}

///
/// @brief 更新下一个栈偏移量
/// @param size
///
void Function::updateNextStackOffset(int64_t size)
{
    nextStackOffset += size;
}

///
/// @brief 设置栈帧大小
///
void Function::setStackFrameSize(int size)
{
    StackFrameSize = size;
}

///
/// @brief 获取栈帧大小
/// @param size
///
int Function::getStackFrameSize()
{
    return StackFrameSize;
}

/// @brief 设置函数调用栈空间大小而引入的栈空间大小
/// @param size 栈空间大小
void Function::setExtraStackSize(int size)
{
    maxExtraStackSize = size;
}