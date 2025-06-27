///
/// @file InstSelectorArm64.cpp
/// @brief 指令选择器-ARM64的实现
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
#include <cstdint>
#include <cstdio>
#include <string>
#include <algorithm> // 用于 std::find
#include <iterator>  // 用于 std::next

#include "Common.h"
#include "ILocArm64.h"
#include "InstSelectorArm64.h"
#include "Instruction.h"
#include "MemVariable.h"
#include "PlatformArm64.h"

#include "PointerType.h"
#include "RegVariable.h"
#include "Function.h"
#include "Value.h"

#include "LabelInstruction.h"
#include "GotoInstruction.h"
#include "BranchInstruction.h"
#include "FuncCallInstruction.h"
#include "MoveInstruction.h"
#include "AllocaInstruction.h"
#include "CFG.h"
#include "VoidType.h"

/// @brief 构造函数
/// @param _irCode 指令
/// @param _iloc ILoc
/// @param _func 函数
InstSelectorArm64::InstSelectorArm64(vector<Instruction *> & _irCode, ILocArm64 & _iloc, Function * _func)
    : ir(_irCode), iloc(_iloc), func(_func)
{
    translator_handlers[IRInstOperator::IRINST_OP_LABEL] = &InstSelectorArm64::translate_label;
    translator_handlers[IRInstOperator::IRINST_OP_GOTO] = &InstSelectorArm64::translate_goto;

    translator_handlers[IRInstOperator::IRINST_OP_ASSIGN] = &InstSelectorArm64::translate_assign;

    translator_handlers[IRInstOperator::IRINST_OP_ADD_I] = &InstSelectorArm64::translate_add_i;
    translator_handlers[IRInstOperator::IRINST_OP_SUB_I] = &InstSelectorArm64::translate_sub_i;
    translator_handlers[IRInstOperator::IRINST_OP_MUL_I] = &InstSelectorArm64::translate_mul_i;
    translator_handlers[IRInstOperator::IRINST_OP_DIV_I] = &InstSelectorArm64::translate_div_i;
    translator_handlers[IRInstOperator::IRINST_OP_MOD_I] = &InstSelectorArm64::translate_mod_i;

    translator_handlers[IRInstOperator::IRINST_OP_FUNC_CALL] = &InstSelectorArm64::translate_call;
    translator_handlers[IRInstOperator::IRINST_OP_ARG] = &InstSelectorArm64::translate_arg;

    translator_handlers[IRInstOperator::IRINST_OP_BR] = &InstSelectorArm64::translate_br;
    translator_handlers[IRInstOperator::IRINST_OP_ALLOCA] = &InstSelectorArm64::translate_alloca;
    translator_handlers[IRInstOperator::IRINST_OP_ICMP] = &InstSelectorArm64::translate_cmp;

    // 添加传统比较指令的翻译
    translator_handlers[IRInstOperator::IRINST_OP_LT] = &InstSelectorArm64::translate_cmp;
    translator_handlers[IRInstOperator::IRINST_OP_LE] = &InstSelectorArm64::translate_cmp;
    translator_handlers[IRInstOperator::IRINST_OP_GT] = &InstSelectorArm64::translate_cmp;
    translator_handlers[IRInstOperator::IRINST_OP_GE] = &InstSelectorArm64::translate_cmp;
    translator_handlers[IRInstOperator::IRINST_OP_EQ] = &InstSelectorArm64::translate_cmp;
    translator_handlers[IRInstOperator::IRINST_OP_NE] = &InstSelectorArm64::translate_cmp;

    // 添加传统分支指令的翻译
    translator_handlers[IRInstOperator::IRINST_OP_BRANCH] = &InstSelectorArm64::translate_br;

    translator_handlers[IRInstOperator::IRINST_OP_LOAD] = &InstSelectorArm64::translate_load;
    translator_handlers[IRInstOperator::IRINST_OP_STORE] = &InstSelectorArm64::translate_store;

    translator_handlers[IRInstOperator::IRINST_OP_RET] = &InstSelectorArm64::translate_ret;

    translator_handlers[IRInstOperator::IRINST_OP_FPTOSI] = &InstSelectorArm64::translate_fptosi;

    // 添加数组相关操作符支持
    translator_handlers[IRInstOperator::IRINST_OP_GEP] = &InstSelectorArm64::translate_gep;
    translator_handlers[IRInstOperator::IRINST_OP_BITCAST] = &InstSelectorArm64::translate_bitcast;
    translator_handlers[IRInstOperator::IRINST_OP_MEMCPY] = &InstSelectorArm64::translate_memcpy;
}

///
/// @brief 析构函数
///
InstSelectorArm64::~InstSelectorArm64()
{}

/// @brief 指令选择执行
void InstSelectorArm64::run()
{
    int i = 1;
    for (auto inst: ir) {
        // 逐个指令进行翻译
        if (!inst->isDead()) {
            translate(inst);
            printf("第%d条指令翻译成功\n", i);
            i++;
        }
    }
}

/// @brief 指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate(Instruction * inst)
{
    if (inst == nullptr) {
        printf("Error: inst is nullptr!\n");
        return;
    }
    // 操作符
    IRInstOperator op = inst->getOp();

    map<IRInstOperator, translate_handler>::const_iterator pIter;
    pIter = translator_handlers.find(op);
    if (pIter == translator_handlers.end()) {
        // 没有找到，则说明当前不支持
        printf("Translate: Operator(%d) not support", (int) op);
        return;
    }

    // 开启时输出IR指令作为注释
    if (showLinearIR) {
        outputIRInstruction(inst);
    }

    (this->*(pIter->second))(inst);
}

///
/// @brief 输出IR指令
///
void InstSelectorArm64::outputIRInstruction(Instruction * inst)
{
    std::string irStr;
    inst->toString(irStr);
    if (!irStr.empty()) {
        iloc.comment(irStr);
    }
}

/// @brief NOP翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_nop(Instruction * inst)
{
    (void) inst;
    iloc.nop();
}

/// @brief Label指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_label(Instruction * inst)
{
    Instanceof(labelInst, LabelInstruction *, inst);

    // 生成符合ARM64标准的标签格式：.L前缀 + 唯一数字标识
    std::string arm64Label = ".L" + labelInst->getIRName();
    iloc.label(arm64Label);
}

/// @brief goto指令指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_goto(Instruction * inst)
{
    if (inst->isDead()) {
        printf("检测到一个br指令为Dead\n");
        return;
    }
    GotoInstruction * gotoInst = dynamic_cast<GotoInstruction *>(inst);
    LabelInstruction * target = gotoInst->getTarget();

    // 检查目标标签是否是下一条指令
    auto it = std::find(ir.begin(), ir.end(), inst);
    if (it != ir.end() && std::next(it) != ir.end()) {
        Instruction * nextInst = *std::next(it);
        if (dynamic_cast<LabelInstruction *>(nextInst) == target) {
            return; // 跳过生成无用跳转
        }
    }

    std::string arm64Label = ".L" + target->getIRName();
    iloc.jump(arm64Label);
}

/// @brief 赋值指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_assign(Instruction * inst)
{
    Value * result = inst->getOperand(0);
    Value * arg1 = inst->getOperand(1);

    /*
     * 函数调用时,为了形式化表达一个活跃区间,引入了结果值为void的MOV
     * 指令，所以这里检测到就不翻译了。
     */
    if (result->getType() == VoidType::getType()) {
        return;
    }

    int32_t arg1_regId = arg1->getRegId();
    int32_t result_regId = result->getRegId();

    if (Instanceof(constVal, ConstInt *, arg1)) {
        // 处理常量的赋值
        if (result_regId != -1) {
            // 常量到寄存器
            iloc.load_imm(result_regId, constVal->getVal());
        } else {
            // 常量到内存
        }
    } else if (arg1_regId != -1) {
        // 寄存器 => 内存
        // 寄存器 => 寄存器

        // 修复：正确的参数顺序应该是 (源寄存器, 目标变量, 临时寄存器)
        iloc.store_var(arg1_regId, result, ARM64_TMP_REG_NO);
    } else if (result_regId != -1) {
        // 内存变量 => 寄存器

        iloc.load_var(result_regId, arg1);
    }
}

/// @brief 二元操作指令翻译成ARM64汇编
/// @param inst IR指令
/// @param operator_name 操作码
/// @param rs_reg_no 结果寄存器号
/// @param op1_reg_no 源操作数1寄存器号
/// @param op2_reg_no 源操作数2寄存器号
void InstSelectorArm64::translate_two_operator(Instruction * inst, string operator_name)
{
    printf("Debug: translate_two_operator - inst=%p, operator=%s\n", inst, operator_name.c_str());

    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    printf("Debug: translate_two_operator - result=%p, arg1=%p, arg2=%p\n", result, arg1, arg2);

    if (!arg1 || !arg2) {
        printf("Error: translate_two_operator - null operand detected\n");
        return;
    }

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = result->getRegId();

    printf("Debug: translate_two_operator - arg1_reg=%d, arg2_reg=%d, result_reg=%d\n",
           arg1_reg_no,
           arg2_reg_no,
           result_reg_no);

    // 检查结果寄存器是否有效
    if (result_reg_no < 0 || result_reg_no >= PlatformArm64::maxRegNum) {
        printf("Error: translate_two_operator - invalid result_reg_no=%d\n", result_reg_no);
        return;
    }

    // 处理操作数：如果不在寄存器中，需要先加载到临时寄存器
    string s1, s2;

    // 处理第一个操作数
    if (Instanceof(constVal, ConstInt *, arg1)) {
        // 操作数1是常量
        if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_I || inst->getOp() == IRInstOperator::IRINST_OP_SUB_I) {
            s1 = "#" + to_string(constVal->getVal());
        } else {
            // 对于其他操作，将常量加载到临时寄存器
            iloc.load_imm(ARM64_TMP_REG_NO, constVal->getVal());
            s1 = PlatformArm64::regName[ARM64_TMP_REG_NO];
        }
    } else if (arg1_reg_no >= 0 && arg1_reg_no < PlatformArm64::maxRegNum) {
        // 操作数1在寄存器中
        s1 = PlatformArm64::regName[arg1_reg_no];
    } else {
        // 操作数1不在寄存器中，需要加载到临时寄存器
        printf("Debug: arg1 not in register, loading to temp register\n");
        iloc.load_var(ARM64_TMP_REG_NO, arg1);
        s1 = PlatformArm64::regName[ARM64_TMP_REG_NO];
    }

    // 处理第二个操作数
    if (Instanceof(constVal, ConstInt *, arg2)) {
        // 操作数2是常量
        if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_I || inst->getOp() == IRInstOperator::IRINST_OP_SUB_I) {
            s2 = "#" + to_string(constVal->getVal());
        } else {
            // 对于其他操作，将常量加载到临时寄存器
            int temp_reg = (s1 == PlatformArm64::regName[ARM64_TMP_REG_NO]) ? ARM64_TMP_REG_NO + 1 : ARM64_TMP_REG_NO;
            iloc.load_imm(temp_reg, constVal->getVal());
            s2 = PlatformArm64::regName[temp_reg];
        }
    } else if (arg2_reg_no >= 0 && arg2_reg_no < PlatformArm64::maxRegNum) {
        // 操作数2在寄存器中
        s2 = PlatformArm64::regName[arg2_reg_no];
    } else {
        // 操作数2不在寄存器中，需要加载到另一个临时寄存器
        printf("Debug: arg2 not in register, loading to temp register\n");
        // 使用不同的临时寄存器避免冲突
        int temp_reg = (s1 == PlatformArm64::regName[ARM64_TMP_REG_NO]) ? ARM64_TMP_REG_NO + 1 : ARM64_TMP_REG_NO;
        iloc.load_var(temp_reg, arg2);
        s2 = PlatformArm64::regName[temp_reg];
    }

    iloc.inst(operator_name, PlatformArm64::regName[result_reg_no], s1, s2);
}

/// @brief 加法指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_add_i(Instruction * inst)
{
    printf("Debug: translate_add_i - inst=%p\n", inst);
    if (inst) {
        printf("Debug: translate_add_i - operands count=%d\n", inst->getOperandsNum());
        if (inst->getOperandsNum() >= 2) {
            Value * arg1 = inst->getOperand(0);
            Value * arg2 = inst->getOperand(1);
            printf("Debug: translate_add_i - arg1=%p, arg2=%p\n", arg1, arg2);
            if (arg1)
                printf("Debug: translate_add_i - arg1 IRName=%s\n", arg1->getIRName().c_str());
            if (arg2)
                printf("Debug: translate_add_i - arg2 IRName=%s\n", arg2->getIRName().c_str());
        }
    }
    translate_two_operator(inst, "add");
}

/// @brief 减法指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_sub_i(Instruction * inst)
{
    translate_two_operator(inst, "subs");
}

/// @brief 乘法指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_mul_i(Instruction * inst)
{
    translate_two_operator(inst, "mul");
}

/// @brief 有符号除法指令翻译为ARM64位汇编
/// @param inst IR指令

void InstSelectorArm64::translate_div_i(Instruction * inst)
{
    translate_two_operator(inst, "sdiv");
}

/// @brief 取余指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_mod_i(Instruction * inst)
{
    // 整数取余：result = arg1 % arg2
    // 实现：result = arg1 - (arg1 / arg2) * arg2
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = result->getRegId();

    // 需要保存被除数的原始值，因为在计算过程中可能被覆盖
    int32_t temp_reg_no = ARM64_TMP_REG_NO;
    bool need_save_arg1 = (arg1_reg_no == result_reg_no);

    // 如果被除数和结果使用同一个寄存器，需要先保存被除数
    if (need_save_arg1) {
        iloc.inst("mov", PlatformArm64::regName[temp_reg_no], PlatformArm64::regName[arg1_reg_no]);
    }

    // 检查是否需要使用临时寄存器来保存除数
    if (arg2_reg_no == result_reg_no) {
        // 除数和结果使用同一个寄存器，需要使用临时寄存器保存除数
        // 注意：如果arg1也需要保存，我们需要另一个临时寄存器
        int32_t temp_reg2_no = need_save_arg1 ? (ARM64_TMP_REG_NO + 1) : ARM64_TMP_REG_NO;

        // 保存除数到临时寄存器
        iloc.inst("mov", PlatformArm64::regName[temp_reg2_no], PlatformArm64::regName[arg2_reg_no]);

        // 计算商：result = arg1 / arg2
        iloc.inst("sdiv",
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[arg1_reg_no],
                  PlatformArm64::regName[temp_reg2_no]);

        // 计算商*除数：result = result * 除数
        iloc.inst("mul",
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[temp_reg2_no]);
    } else {
        // 除数和结果使用不同寄存器，可以直接计算

        // 计算商：result = arg1 / arg2
        iloc.inst("sdiv",
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[arg1_reg_no],
                  PlatformArm64::regName[arg2_reg_no]);

        // 计算商*除数：result = result * arg2
        iloc.inst("mul",
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[arg2_reg_no]);
    }

    // 计算余数：result = arg1 - result
    // 如果之前保存了arg1，使用保存的值
    std::string arg1_reg_name =
        need_save_arg1 ? PlatformArm64::regName[temp_reg_no] : PlatformArm64::regName[arg1_reg_no];

    iloc.inst("subs", PlatformArm64::regName[result_reg_no], arg1_reg_name, PlatformArm64::regName[result_reg_no]);
}

/// @brief 函数调用指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_call(Instruction * inst)
{
    FuncCallInstruction * callInst = dynamic_cast<FuncCallInstruction *>(inst);

    int32_t operandNum = callInst->getOperandsNum() - 1;
    if (callInst->hasResultValue()) {
        operandNum--;
    }
    printf("callInst_operandNum：%d\n", operandNum);

    if (operandNum != realArgCount) {

        // 两者不一致 也可能没有ARG指令，正常
        if (realArgCount != 0) {

            minic_log(LOG_ERROR, "ARG指令的个数与调用函数个数不一致");
        }
    }

    iloc.call_fun(callInst->getName());

    // 函数调用后清零，使得下次可正常统计
    realArgCount = 0;
}

///
/// @brief 实参指令翻译成ARM64汇编
/// @param inst
///
void InstSelectorArm64::translate_arg(Instruction * inst)
{
    // 翻译之前必须确保源操作数要么是寄存器，要么是内存，否则出错。
    printf("开始翻译ARG指令, realArgCount：%d\n", realArgCount);
    Value * src = inst->getOperand(0);

    // 当前统计的ARG指令个数
    int32_t regId = src->getRegId();

    if (realArgCount < 8) {
        // 前八个参数通过寄存器传递
        if (regId != -1) {
            if (regId != realArgCount) {
                // 肯定寄存器分配有误
                minic_log(LOG_ERROR, "第%d个ARG指令对象寄存器分配有误: %d", argCount + 1, regId);
            } else {
                // 生成将源操作数移动到对应寄存器的指令
                iloc.mov_reg(realArgCount, regId);
            }
        } else {
            minic_log(LOG_ERROR, "第%d个ARG指令对象不是寄存器", argCount + 1);
        }
    } else {
        // 超过八个的参数通过栈传递
        int32_t baseRegId;
        bool result = src->getMemoryAddr(&baseRegId);
        if ((!result) || (baseRegId != ARM64_SP_REG_NO)) {
            minic_log(LOG_ERROR, "第%d个ARG指令对象不是SP寄存器寻址", argCount + 1);
        } else {
            // 计算栈偏移
            int64_t offset = (realArgCount - 8) * int64_t(8);
            // 生成将源操作数存储到栈上的指令
            iloc.store_base(regId, ARM64_SP_REG_NO, offset, -1);
        }
    }

    realArgCount++;
}

///
/// @brief 条件分支跳=转指令翻译成ARM64汇编
/// @param inst
///
void InstSelectorArm64::translate_br(Instruction * inst)
{
    // 尝试转换为 BranchInstruction
    BranchInstruction * branchInst = dynamic_cast<BranchInstruction *>(inst);
    if (!branchInst) {
        printf("Error: Not a BranchInstruction\n");
        return;
    }

    // 获取条件操作数和标签
    Value * cond = branchInst->getCondition();
    LabelInstruction * iftrue = branchInst->getTrueLabel();
    LabelInstruction * iffalse = branchInst->getFalseLabel();

    // 检查指针是否有效
    if (!cond || !iftrue || !iffalse) {
        printf("Error: Invalid operands in br instruction\n");
        return;
    }

    // 获取条件操作数分配的寄存器号
    int32_t cond_reg_no = cond->getRegId();
    printf("Debug: br instruction cond_reg_no = %d\n", cond_reg_no);

    if (cond_reg_no == -1) {
        printf("Error: Condition operand not allocated to register\n");
        return;
    }

    // 生成符合标准的标签格式
    std::string trueLabel = ".L" + iftrue->getIRName();
    std::string falseLabel = ".L" + iffalse->getIRName();
    printf("Debug: trueLabel = %s, falseLabel = %s\n", trueLabel.c_str(), falseLabel.c_str());

    iloc.inst("cmp", PlatformArm64::regName[cond_reg_no], "#0");
    iloc.inst("b.ne", trueLabel); // 使用标准标签格式
    iloc.jump(falseLabel); // 使用标准标签格式                         // 条件为0,跳转到iffalse标签
}

///
/// @brief 栈分配指令翻译成ARM64位汇编
/// @param inst
///
void InstSelectorArm64::translate_alloca(Instruction * inst)
{
    // 在ARM64中，栈空间在函数入口处一次性分配
    // alloca指令在栈分配阶段已处理（stackAlloc函数中）
    // 此处不需要生成实际汇编指令，仅需确保变量已在栈帧中分配空间

    // 获取alloca指令的目标变量（即分配的栈空间地址）
    Value * result = inst;

    // 验证变量是否已在栈上分配空间
    int32_t baseRegId;
    int64_t offset;
    if (!result->getMemoryAddr(&baseRegId, &offset)) {
        // 若未分配，记录错误（正常情况下不应发生）
        minic_log(LOG_ERROR, "Alloca variable not allocated on stack: %s", result->getName().c_str());
    }

    // alloca指令不应该生成地址加载指令
    // alloca的作用是分配栈空间，其结果是一个内存地址，不需要加载到寄存器
    // 如果alloca指令被错误地分配了寄存器ID，我们应该忽略它
    if (result->getRegId() != -1) {
        printf("Warning: alloca指令 %s 被错误地分配了寄存器ID=%d，忽略地址加载\n",
               result->getIRName().c_str(),
               result->getRegId());
        // 不生成地址加载指令，因为alloca的结果应该是栈地址，不是寄存器值
    }

    // 调试输出：标记alloca指令已处理
    if (showLinearIR) {
        std::string comment = "; alloca processed: " + result->getName() + " at [FP" + (offset >= 0 ? "+" : "") +
                              std::to_string(offset) + "]";
        iloc.comment(comment);
    }
}

///
/// @brief 比较指令翻译成ARM64位汇编
/// @param inst
///
void InstSelectorArm64::translate_cmp(Instruction * inst)
{
    // 获取操作数
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);
    Value * result = inst;

    // 获取操作数的寄存器编号
    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = result->getRegId();

    // 处理常量操作数
    std::string arg1_str = PlatformArm64::regName[arg1_reg_no];
    std::string arg2_str;

    if (Instanceof(constVal, ConstInt *, arg2)) {
        // 第二个操作数是常量
        arg2_str = "#" + std::to_string(constVal->getVal());
    } else {
        arg2_str = PlatformArm64::regName[arg2_reg_no];
    }

    // 生成比较指令
    iloc.inst("cmp", arg1_str, arg2_str);

    // 根据比较类型设置结果寄存器
    IRInstOperator op = inst->getOp();
    std::string condition;

    switch (op) {
        case IRInstOperator::IRINST_OP_LT:
        case IRInstOperator::IRINST_OP_ICMP: // 假设 icmp slt
            condition = "lt";
            break;
        case IRInstOperator::IRINST_OP_LE:
            condition = "le";
            break;
        case IRInstOperator::IRINST_OP_GT:
            condition = "gt";
            break;
        case IRInstOperator::IRINST_OP_GE:
            condition = "ge";
            break;
        case IRInstOperator::IRINST_OP_EQ:
            condition = "eq";
            break;
        case IRInstOperator::IRINST_OP_NE:
            condition = "ne";
            break;
        default:
            condition = "lt"; // 默认为小于
            break;
    }

    // 设置结果寄存器：如果条件成立则为1，否则为0
    // 使用 cset 指令根据条件设置结果
    iloc.inst("cset", PlatformArm64::regName[result_reg_no], condition);
}

/// @brief load指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_load(Instruction * inst)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);

    int32_t result_regId = result->getRegId();

    printf("Debug: translate_load - result=%p, arg1=%p\n", result, arg1);
    printf("Debug: translate_load - result_regId=%d, arg1_regId=%d\n", result_regId, arg1->getRegId());

    if (arg1 == nullptr) {
        printf("Error: load instruction operand is null\n");
        return;
    }

    printf("Debug: translate_load - arg1 name=%s, IRName=%s\n", arg1->getName().c_str(), arg1->getIRName().c_str());

    if (result_regId != -1) {
        // 内存变量 => 寄存器
        iloc.load_var(result_regId, arg1);
    } else {
        // 若结果变量不是寄存器，分配一个新的寄存器来保存加载的结果
        // TODO 可能需要在寄存器分配前检查load指令结果变量是否为寄存器，若不是则插入赋值语句
        // int32_t temp_regno = simpleRegisterAllocator.Allocate(result);
        // iloc.load_var(temp_regno, arg1);
        // result->setLoadRegId(temp_regno);
        // 后续可以考虑将结果保存到合适的位置，这里暂时不做处理
        // simpleRegisterAllocator.free(temp_regno);
    }
}

/// @brief store指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_store(Instruction * inst)
{
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_regId = arg1->getRegId();

    // 优先检查是否是常量0，即使它被分配了寄存器
    ConstInt * constVal = dynamic_cast<ConstInt *>(arg1);
    if (constVal && constVal->getVal() == 0) {
        // 常量0使用零寄存器，更高效
        int32_t dest_baseRegId = -1;
        int64_t dest_offset = -1;
        arg2->getMemoryAddr(&dest_baseRegId, &dest_offset);
        std::string s = "[" + PlatformArm64::regName[dest_baseRegId] + ",#" + std::to_string(dest_offset) + "]";
        iloc.inst("str", "wzr", s);
    } else if (arg1_regId != -1) {
        // 寄存器 => 内存
        printf("寄存器 => 内存");
        iloc.store_var(arg1_regId, arg2, ARM64_TMP_REG_NO);
    } else {
        // 若源操作数不是寄存器，先加载到一个临时寄存器
        if (Instanceof(constVal, ConstInt *, arg1)) {
            // 其他整数常量情况
            // TODO 可能需要在寄存器分配前检查store指令源操作数是否为寄存器，若不是则插入赋值语句
            // int32_t temp_regno = simpleRegisterAllocator.Allocate(arg1);
            // iloc.load_var(temp_regno, arg1);
            // iloc.store_var(temp_regno, arg2, ARM64_TMP_REG_NO);
            // simpleRegisterAllocator.free(temp_regno);
        }

        // TODO浮点情况
    }
}

/// @brief ret指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_ret(Instruction * inst)
{
    /*Function * func = this->func;
    Value * returnValue = func->getReturnValue();

    if (returnValue != nullptr) {
        int32_t resultRegId = returnValue->getRegId();

        // 如果返回值不在w0，将其移动到w0
        if (resultRegId != 0) {
            // 使用mov指令将x0的低32位移动到w0（适用于32位返回值）
            // 或直接使用mov将64位值移动到w0（若返回值为64位但需截断）
            iloc.inst("mov", PlatformArm64::regName[0], PlatformArm64::regName[resultRegId]);
        }
    }*/
    // Value * returnValue = func->getReturnValue();
    Value * returnValue;
    // printval(returnValue);
    // 如果存在返回值，确保其位于x0寄存器
    // if (returnValue != nullptr) {
    if (inst->getOperandsNum()) {
        returnValue = inst->getOperand(0);
        int32_t resultRegId = returnValue->getRegId();
        /*TODO 在汇编阶段临时添加MOV原则上是不行的（至少在这个项目里），
         * 因为MOV到的寄存器未必空闲，所以应当在寄存器分配前就加好MOV指令
         */
        // 如果返回值未在x0中，进行寄存器移动
        if (resultRegId != 0) {
            printf("返回值未在x0中，进行寄存器移动:%d\n", resultRegId);
            if (resultRegId != -1) {
                iloc.inst("mov", PlatformArm64::regName[0], PlatformArm64::regName[resultRegId]);
            } else {
                LocalVariable * localResult = dynamic_cast<LocalVariable *>(returnValue);
                int off = localResult->getOffset();
                std::string s = "[sp,#" + std::to_string(off) + "]";
                iloc.inst("ldr", PlatformArm64::regName[0], s);
            }
        }
    }

    iloc.emitFunctionEpilogue(func);
}

/// @brief fptosi指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_fptosi(Instruction * inst)
{}

/// @brief getelementptr指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_gep(Instruction * inst)
{
    // getelementptr指令用于计算数组元素的地址
    // 格式: result = getelementptr type, type* ptr, i64 index1, i64 index2, ...

    Value * result = inst;
    Value * basePtr = inst->getOperand(0); // 基址指针

    int32_t result_reg = result->getRegId();

    // 如果结果需要在寄存器中，我们需要计算地址
    if (result_reg != -1) {
        // 对于数组访问，我们需要计算偏移量
        // 这里简化处理：如果基址在内存中，我们计算其地址
        int32_t base_reg_id;
        int64_t base_offset;

        if (basePtr->getMemoryAddr(&base_reg_id, &base_offset)) {
            // 基址在栈上，计算其地址
            printf("Debug: getelementptr basePtr=%s, base_offset = %ld, base_reg_id = %d\n",
                   basePtr->getIRName().c_str(),
                   base_offset,
                   base_reg_id);
            if (inst->getOperandsNum() >= 3) {
                // 有索引，需要计算偏移
                // 简化处理：对于二维数组 arr[i][j]，偏移 = base_offset + i*sizeof(row) + j*sizeof(element)
                // 这里我们暂时只处理常量索引

                printf("Debug: getelementptr operands count = %d\n", inst->getOperandsNum());
                for (int i = 0; i < inst->getOperandsNum(); i++) {
                    Value * op = inst->getOperand(i);
                    printf("Debug: operand[%d] = %s\n", i, op ? op->getIRName().c_str() : "null");
                }

                // 获取第一个索引（通常是0，表示数组本身）
                Value * index1 = inst->getOperand(1);
                (void) index1; // 避免未使用变量警告

                // 检查实际的操作数数量（排除结果）
                int actual_operands = inst->getOperandsNum();
                // 如果最后一个操作数是指令本身的结果，则排除它
                if (actual_operands > 0 && inst->getOperand(actual_operands - 1) == inst) {
                    actual_operands--;
                }

                if (actual_operands == 3) {
                    // 一维数组访问：arr[index]
                    Value * index = inst->getOperand(2);
                    ConstInt * constIdx = dynamic_cast<ConstInt *>(index);
                    if (constIdx) {
                        int64_t idx = constIdx->getVal();
                        int64_t element_offset = base_offset + (idx * 4); // 假设int类型，4字节

                        printf("Debug: 1D array access: idx=%ld, base_offset=%ld, element_offset=%ld\n",
                               idx,
                               base_offset,
                               element_offset);

                        // 生成地址计算指令
                        std::string result_reg_name = PlatformArm64::regName[result_reg];
                        std::string base_reg_name = PlatformArm64::regName[base_reg_id];

                        // 确保使用64位寄存器
                        if (result_reg_name[0] == 'w') {
                            result_reg_name[0] = 'x';
                        }
                        if (base_reg_name[0] == 'w') {
                            base_reg_name[0] = 'x';
                        }

                        // 处理偏移量
                        if (element_offset >= 0) {
                            iloc.inst("add", result_reg_name, base_reg_name, "#" + std::to_string(element_offset));
                        } else {
                            iloc.inst("sub", result_reg_name, base_reg_name, "#" + std::to_string(-element_offset));
                        }
                        return;
                    }
                } else if (actual_operands >= 4) {
                    // 获取第二个索引（行索引）
                    Value * index2 = inst->getOperand(2);
                    if (inst->getOperandsNum() >= 5) {
                        // 获取第三个索引（列索引）
                        Value * index3 = inst->getOperand(3);

                        // 对于int[4][2]数组，每行8字节，每个元素4字节
                        ConstInt * constIdx2 = dynamic_cast<ConstInt *>(index2);
                        ConstInt * constIdx3 = dynamic_cast<ConstInt *>(index3);
                        if (constIdx2 && constIdx3) {
                            int64_t row_idx = constIdx2->getVal();
                            int64_t col_idx = constIdx3->getVal();
                            int64_t element_offset = base_offset + (row_idx * 8) + (col_idx * 4);

                            // printf("Debug: getelementptr element calculation: row_idx=%ld, col_idx=%ld, "
                            //        "element_offset=%ld\n",
                            //        row_idx,
                            //        col_idx,
                            //        element_offset);

                            // 生成地址计算指令: add result_reg, sp, #offset
                            // 注意：在ARM64中，地址计算必须使用64位寄存器
                            std::string result_reg_name = PlatformArm64::regName[result_reg];
                            std::string base_reg_name = PlatformArm64::regName[base_reg_id];

                            // 如果是32位寄存器名，转换为64位
                            if (result_reg_name[0] == 'w') {
                                result_reg_name[0] = 'x';
                            }
                            if (base_reg_name[0] == 'w') {
                                base_reg_name[0] = 'x';
                            }

                            // 处理负偏移量
                            if (element_offset >= 0) {
                                iloc.inst("add", result_reg_name, base_reg_name, "#" + std::to_string(element_offset));
                            } else {
                                iloc.inst("sub", result_reg_name, base_reg_name, "#" + std::to_string(-element_offset));
                            }
                            return;
                        }
                    }
                }
            }

            // 默认情况：只是获取数组的基地址
            // 注意：在ARM64中，地址计算必须使用64位寄存器
            std::string result_reg_name = PlatformArm64::regName[result_reg];
            std::string base_reg_name = PlatformArm64::regName[base_reg_id];

            // 如果是32位寄存器名，转换为64位
            if (result_reg_name[0] == 'w') {
                result_reg_name[0] = 'x';
            }
            if (base_reg_name[0] == 'w') {
                base_reg_name[0] = 'x';
            }

            // 处理负偏移量
            if (base_offset >= 0) {
                iloc.inst("add", result_reg_name, base_reg_name, "#" + std::to_string(base_offset));
            } else {
                iloc.inst("sub", result_reg_name, base_reg_name, "#" + std::to_string(-base_offset));
            }
        } else {
            // 检查是否是全局变量
            if (auto globalVar = dynamic_cast<GlobalVariable *>(basePtr)) {
                // 处理全局变量的GEP指令
                printf("Debug: getelementptr for global variable %s\n", globalVar->getName().c_str());

                // 获取索引值来计算偏移量
                if (inst->getOperandsNum() >= 3) {
                    Value * index1 = inst->getOperand(1); // 第一个索引（通常是0）
                    Value * index2 = inst->getOperand(2); // 第二个索引（数组元素索引）

                    ConstInt * constIdx1 = dynamic_cast<ConstInt *>(index1);
                    ConstInt * constIdx2 = dynamic_cast<ConstInt *>(index2);

                    if (constIdx1 && constIdx2) {
                        int64_t idx1 = constIdx1->getVal();
                        int64_t idx2 = constIdx2->getVal();

                        // 对于一维数组 a[5]，偏移量 = idx2 * sizeof(element)
                        // 假设是int数组，每个元素4字节
                        int64_t element_offset = idx2 * 4;

                        printf("Debug: Global array access: idx1=%ld, idx2=%ld, element_offset=%ld\n",
                               idx1,
                               idx2,
                               element_offset);

                        // 生成地址计算指令
                        std::string result_reg_name = PlatformArm64::regName[result_reg];
                        if (result_reg_name[0] == 'w') {
                            result_reg_name[0] = 'x';
                        }

                        // 加载全局变量的基地址
                        // adrp x_reg, symbol
                        iloc.inst("adrp", result_reg_name, globalVar->getName());
                        // add x_reg, x_reg, :lo12:symbol
                        iloc.inst("add", result_reg_name, result_reg_name, ":lo12:" + globalVar->getName());

                        // 如果有偏移量，添加偏移
                        if (element_offset > 0) {
                            iloc.inst("add", result_reg_name, result_reg_name, "#" + std::to_string(element_offset));
                        }

                        return;
                    }
                }

                // 如果没有索引或索引不是常量，只加载基地址
                std::string result_reg_name = PlatformArm64::regName[result_reg];
                if (result_reg_name[0] == 'w') {
                    result_reg_name[0] = 'x';
                }
                // adrp x_reg, symbol
                iloc.inst("adrp", result_reg_name, globalVar->getName());
                // add x_reg, x_reg, :lo12:symbol
                iloc.inst("add", result_reg_name, result_reg_name, ":lo12:" + globalVar->getName());
            } else {
                // 基址在寄存器中，直接复制
                int32_t base_reg = basePtr->getRegId();
                if (base_reg != -1 && base_reg != result_reg) {
                    iloc.inst("mov", PlatformArm64::regName[result_reg], PlatformArm64::regName[base_reg]);
                }
            }
        }
    }
}

/// @brief bitcast指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_bitcast(Instruction * inst)
{
    // bitcast指令用于类型转换，但不改变位模式
    // 在ARM64中，对于指针类型的bitcast，通常不需要生成实际指令
    // 只需要确保寄存器分配正确

    Value * result = inst;
    Value * source = inst->getOperand(0);

    int32_t result_reg = result->getRegId();
    int32_t source_reg = source->getRegId();

    if (result_reg != -1) {
        if (source_reg != -1) {
            // 源和目标都在寄存器中
            if (result_reg != source_reg) {
                // bitcast通常用于地址转换，确保使用64位寄存器
                std::string result_reg_name = PlatformArm64::regName[result_reg];
                std::string source_reg_name = PlatformArm64::regName[source_reg];
                if (result_reg_name[0] == 'w') {
                    result_reg_name[0] = 'x';
                }
                if (source_reg_name[0] == 'w') {
                    source_reg_name[0] = 'x';
                }
                iloc.inst("mov", result_reg_name, source_reg_name);
            }
        } else {
            // 源不在寄存器中，可能是全局变量或局部变量
            if (auto globalVar = dynamic_cast<GlobalVariable *>(source)) {
                // 加载全局变量地址
                // adrp指令必须使用64位寄存器
                std::string result_reg_name = PlatformArm64::regName[result_reg];
                if (result_reg_name[0] == 'w') {
                    result_reg_name[0] = 'x';
                }
                iloc.inst("adrp", result_reg_name, globalVar->getName());
                iloc.inst("add", result_reg_name, result_reg_name, ":lo12:" + globalVar->getName());
            } else {
                // 其他情况，加载局部变量地址
                int32_t base_reg_id;
                int64_t offset;
                if (source->getMemoryAddr(&base_reg_id, &offset)) {
                    // 使用lea_var加载变量地址
                    iloc.lea_var(result_reg, source);
                } else {
                    printf("Error: bitcast source has no memory address\n");
                }
            }
        }
    }
}

/// @brief memcpy指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_memcpy(Instruction * inst)
{
    // memcpy指令用于内存复制
    // 参数：dest, src, size, volatile
    Value * dest = inst->getOperand(0);
    Value * src = inst->getOperand(1);
    Value * size = inst->getOperand(2);

    // 获取目标和源地址的寄存器
    int32_t dest_reg = dest->getRegId();
    int32_t src_reg = src->getRegId();

    // 确保地址在寄存器中
    if (dest_reg == -1 || src_reg == -1) {
        printf("Error: memcpy operands not in registers\n");
        return;
    }

    // 获取复制大小
    if (auto constSize = dynamic_cast<ConstInt *>(size)) {
        int copySize = constSize->getVal();
        int wordCount = (copySize + 3) / 4; // 向上取整到字边界

        // 使用循环复制数据
        for (int i = 0; i < wordCount; i++) {
            // 从源地址加载数据到临时寄存器
            std::string src_reg_name = PlatformArm64::regName[src_reg];
            std::string dest_reg_name = PlatformArm64::regName[dest_reg];

            // 确保使用64位寄存器进行地址计算
            if (src_reg_name[0] == 'w') {
                src_reg_name[0] = 'x';
            }
            if (dest_reg_name[0] == 'w') {
                dest_reg_name[0] = 'x';
            }

            // 从源地址加载数据
            iloc.inst("ldr", "w2", "[" + src_reg_name + ", #" + std::to_string(i * 4) + "]");
            // 存储到目标地址
            iloc.inst("str", "w2", "[" + dest_reg_name + ", #" + std::to_string(i * 4) + "]");
        }
    } else {
        // 动态大小的memcpy，暂时不实现
        printf("Warning: Dynamic size memcpy not implemented\n");
    }
}