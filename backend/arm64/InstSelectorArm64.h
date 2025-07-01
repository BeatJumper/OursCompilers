///
/// @file InstSelectorArm32.h
/// @brief 指令选择器-ARM32
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
#pragma once

#include <map>
#include <vector>
#include "BitMap.h"
#include "Value.h"
#include "PlatformArm64.h"

#include "Function.h"
#include "ILocArm64.h"
#include "Instruction.h"
#include "PlatformArm64.h"
#include "RegVariable.h"

using namespace std;

/// @brief 指令选择器-ARM64
class InstSelectorArm64 {

    /// @brief 所有的IR指令
    std::vector<Instruction *> & ir;

    /// @brief 指令变换
    ILocArm64 & iloc;

    /// @brief 要处理的函数
    Function * func;

protected:
    /// @brief 指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate(Instruction * inst);

    /// @brief NOP翻译成ARM64汇编
    /// @param inst IR指令
    void translate_nop(Instruction * inst);

    /// @brief 函数入口指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_entry(Instruction * inst);

    /// @brief 函数出口指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_exit(Instruction * inst);

    /// @brief 赋值指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_assign(Instruction * inst);

    /// @brief Label指令指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_label(Instruction * inst);

    /// @brief goto指令指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_goto(Instruction * inst);

    /// @brief 整数加法指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_add_i(Instruction * inst);

    /// @brief 整数减法指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_sub_i(Instruction * inst);

    /// @brief 乘法指令翻译成ARiM64汇编
    /// @param inst IR指令
    void translate_mul_i(Instruction * inst);

    /// @brief 取余指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_mod_i(Instruction * inst);

    /// @brief 有符号除法指令翻译为ARM64位汇编
    /// @param inst IR指令
    void translate_div_i(Instruction * inst);

    /// @brief 浮点数加法指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_add_f(Instruction * inst);

    /// @brief 浮点数减法指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_sub_f(Instruction * inst);

    /// @brief 浮点数乘法指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_mul_f(Instruction * inst);

    /// @brief 浮点数除法指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_div_f(Instruction * inst);

    /// @brief 二元操作指令翻译成ARM64汇编
    /// @param inst IR指令
    /// @param operator_name 操作码
    void translate_two_operator(Instruction * inst, string operator_name);

    /// @brief 浮点数二元操作指令翻译成ARM64汇编
    /// @param inst IR指令
    /// @param operator_name 操作码
    void translate_two_operator_float(Instruction * inst, string operator_name);

    /// @brief 函数调用指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_call(Instruction * inst);

    ///
    /// @brief 实参指令翻译成ARM64汇编
    /// @param inst
    ///
    void translate_arg(Instruction * inst);

    ///
    /// @brief 分支指令翻译成ARM64汇编
    /// @param inst
    ///
    void translate_br(Instruction * inst);

    ///
    /// @brief 全局变量定义翻译成ARM64位汇编
    /// @param inst
    ///
    void translate_global(Instruction * inst);

    ///
    /// @brief 栈分配指令翻译成ARM64位汇编
    /// @param inst
    ///
    void translate_alloca(Instruction * inst);

    ///
    /// @brief 比较指令翻译成ARM64位汇编
    /// @param inst
    ///
    void translate_cmp(Instruction * inst);

    ///
    /// @brief load指令翻译成ARM64汇编
    /// @param inst
    ///
    void translate_load(Instruction * inst);

    ///
    /// @brief store指令翻译成ARM64汇编
    /// @param inst IR指令
    ///
    void translate_store(Instruction * inst);

    ///
    /// @brief ret指令翻译成ARM64汇编
    /// @param inst IR指令
    ///
    void translate_ret(Instruction * inst);

    ///
    /// @brief fptosi指令翻译成ARM64汇编
    /// @param inst IR指令
    ///
    void translate_fptosi(Instruction * inst);

    ///
    /// @brief sitofp指令翻译成ARM64汇编
    /// @param inst IR指令
    ///
    void translate_sitofp(Instruction * inst);

    ///
    /// @brief getelementptr指令翻译成ARM64汇编
    /// @param inst IR指令
    ///
    void translate_gep(Instruction * inst);

    ///
    /// @brief bitcast指令翻译成ARM64汇编
    /// @param inst IR指令
    ///
    void translate_bitcast(Instruction * inst);

    ///
    /// @brief memcpy指令翻译成ARM64汇编
    /// @param inst IR指令
    ///
    void translate_memcpy(Instruction * inst);

    ///
    /// @brief memset指令翻译成ARM64汇编
    /// @param inst IR指令
    ///
    void translate_memset(Instruction * inst);

    /// @brief sext指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_sext(Instruction * inst);

    /// @brief zext指令翻译成ARM64汇编
    /// @param inst IR指令
    void translate_zext(Instruction * inst);

    ///
    /// @brief 输出IR指令
    ///
    void outputIRInstruction(Instruction * inst);

    /// @brief IR翻译动作函数原型
    typedef void (InstSelectorArm64::*translate_handler)(Instruction *);

    /// @brief IR动作处理函数清单
    map<IRInstOperator, translate_handler> translator_handlers;

    ///
    /// @brief 函数实参累计
    ///
    int32_t argCount = 0;

    /// @brief 累计的实参个数
    int32_t realArgCount = 0;

    ///
    /// @brief 显示IR指令内容
    ///
    bool showLinearIR = false;

public:
    /// @brief 构造函数
    /// @param _irCode IR指令
    /// @param _func 函数
    /// @param _iloc 后端指令
    InstSelectorArm64(std::vector<Instruction *> & _irCode, ILocArm64 & _iloc, Function * _func);

    ///
    /// @brief 析构函数
    ///
    ~InstSelectorArm64();

    ///
    /// @brief 设置是否输出线性IR的内容
    /// @param show true显示，false显示
    ///
    void setShowLinearIR(bool show)
    {
        showLinearIR = show;
    }

    /// @brief 指令选择
    void run();
};
