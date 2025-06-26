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
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = result->getRegId();

    string s1 = PlatformArm64::regName[arg1_reg_no];
    string s2 = PlatformArm64::regName[arg2_reg_no];

    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (Instanceof(constVal, ConstInt *, arg1)) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_I || inst->getOp() == IRInstOperator::IRINST_OP_SUB_I) {
            s1 = to_string(constVal->getVal());
        }
    }

    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (Instanceof(constVal, ConstInt *, arg2)) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_ADD_I || inst->getOp() == IRInstOperator::IRINST_OP_SUB_I) {
            s2 = to_string(constVal->getVal());
        }
    }

    iloc.inst(operator_name, PlatformArm64::regName[result_reg_no], s1, s2);
}

/// @brief 加法指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_add_i(Instruction * inst)
{
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

    // 检查是否需要使用临时寄存器来保存除数
    if (arg2_reg_no == result_reg_no) {
        // 除数和结果使用同一个寄存器，需要使用临时寄存器保存除数
        int32_t temp_reg_no = ARM64_TMP_REG_NO;

        // 保存除数到临时寄存器
        iloc.inst("mov", PlatformArm64::regName[temp_reg_no], PlatformArm64::regName[arg2_reg_no]);

        // 计算商：result = arg1 / arg2
        iloc.inst("sdiv",
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[arg1_reg_no],
                  PlatformArm64::regName[temp_reg_no]);

        // 计算商*除数：result = result * 除数
        iloc.inst("mul",
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[result_reg_no],
                  PlatformArm64::regName[temp_reg_no]);
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
    iloc.inst("subs",
              PlatformArm64::regName[result_reg_no],
              PlatformArm64::regName[arg1_reg_no],
              PlatformArm64::regName[result_reg_no]);
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

    // 如果需要，可以生成加载变量地址的指令
    if (result->getRegId() != -1) {
        // 如果结果需要加载到寄存器
        iloc.lea_var(result->getRegId(), result);
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

    if (arg1_regId != -1) {
        // 寄存器 => 内存
        printf("寄存器 => 内存");
        iloc.store_var(arg1_regId, arg2, ARM64_TMP_REG_NO);
    } else {
        // 若源操作数不是寄存器，先加载到一个临时寄存器
        if (Instanceof(constVal, ConstInt *, arg1)) {
            // 整数情况
            if (constVal->getVal() == 0) {
                int32_t dest_baseRegId = -1;
                int64_t dest_offset = -1;
                arg2->getMemoryAddr(&dest_baseRegId, &dest_offset);
                std::string s = "[" + PlatformArm64::regName[dest_baseRegId] + ",#" + std::to_string(dest_offset) + "]";
                iloc.inst("str", "wzr", s);
            } else {
                // TODO 可能需要在寄存器分配前检查store指令源操作数是否为寄存器，若不是则插入赋值语句
                // int32_t temp_regno = simpleRegisterAllocator.Allocate(arg1);
                // iloc.load_var(temp_regno, arg1);
                // iloc.store_var(temp_regno, arg2, ARM64_TMP_REG_NO);
                // simpleRegisterAllocator.free(temp_regno);
            }
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
    Value * returnValue = func->getReturnValue();

    // 如果存在返回值，确保其位于x0寄存器
    if (returnValue != nullptr) {
        int32_t resultRegId = returnValue->getRegId();

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