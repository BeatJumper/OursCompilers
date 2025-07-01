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
#include "ArrayType.h"
#include "IntegerType.h"
#include "RegVariable.h"
#include "Function.h"
#include "Value.h"

#include "LabelInstruction.h"
#include "GotoInstruction.h"
#include "BranchInstruction.h"
#include "FuncCallInstruction.h"
#include "MoveInstruction.h"
#include "AllocaInstruction.h"
#include "LoadInstruction.h"
#include "GetelementptrInstruction.h"
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

    translator_handlers[IRInstOperator::IRINST_OP_SEXT] = &InstSelectorArm64::translate_sext;
    translator_handlers[IRInstOperator::IRINST_OP_ZEXT] = &InstSelectorArm64::translate_zext;

    // 浮点数算术指令
    translator_handlers[IRInstOperator::IRINST_OP_ADD_F] = &InstSelectorArm64::translate_add_f;
    translator_handlers[IRInstOperator::IRINST_OP_SUB_F] = &InstSelectorArm64::translate_sub_f;
    translator_handlers[IRInstOperator::IRINST_OP_MUL_F] = &InstSelectorArm64::translate_mul_f;
    translator_handlers[IRInstOperator::IRINST_OP_DIV_F] = &InstSelectorArm64::translate_div_f;

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
    translator_handlers[IRInstOperator::IRINST_OP_SITOFP] = &InstSelectorArm64::translate_sitofp;

    // 添加数组相关操作符支持
    translator_handlers[IRInstOperator::IRINST_OP_GEP] = &InstSelectorArm64::translate_gep;
    translator_handlers[IRInstOperator::IRINST_OP_BITCAST] = &InstSelectorArm64::translate_bitcast;
    translator_handlers[IRInstOperator::IRINST_OP_MEMCPY] = &InstSelectorArm64::translate_memcpy;
    translator_handlers[IRInstOperator::IRINST_OP_MEMSET] = &InstSelectorArm64::translate_memset;
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
            printf("第%d条指令翻译成功,寄存器编号:%d\n", i, inst->getRegId());
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

    if (!arg1 || !arg2) {
        printf("Error: translate_two_operator - null operand detected\n");
        return;
    }

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = result->getRegId();

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

/// @brief 浮点数二元操作指令翻译成ARM64汇编
/// @param inst IR指令
/// @param operator_name 操作码
void InstSelectorArm64::translate_two_operator_float(Instruction * inst, string operator_name)
{
    // 获取操作数
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    // 获取寄存器编号
    int result_reg_no = inst->getRegId();
    int arg1_reg_no = arg1->getRegId();
    int arg2_reg_no = arg2->getRegId();

    printf("Debug: translate_two_operator_float - result_reg=%d, arg1_reg=%d, arg2_reg=%d\n",
           result_reg_no,
           arg1_reg_no,
           arg2_reg_no);

    // 检查结果寄存器编号有效性
    if (result_reg_no < 0 || result_reg_no >= PlatformArm64::maxVecRegNum) {
        printf("Error: Invalid result register number: %d\n", result_reg_no);
        return;
    }

    // 处理操作数
    std::string s1, s2;

    // 处理第一个操作数
    if (arg1_reg_no >= 0 && arg1_reg_no < PlatformArm64::maxVecRegNum) {
        // 操作数1在浮点寄存器中
        s1 = PlatformArm64::floatRegName[arg1_reg_no];
    } else {
        // 操作数1不在寄存器中，需要加载到临时寄存器
        printf("Debug: arg1 not in register, loading to temp register\n");
        iloc.load_var(ARM64_TMP_REG_NO, arg1);
        s1 = PlatformArm64::regName[ARM64_TMP_REG_NO];
    }

    // 处理第二个操作数
    if (arg2_reg_no >= 0 && arg2_reg_no < PlatformArm64::maxVecRegNum) {
        // 操作数2在浮点寄存器中
        s2 = PlatformArm64::floatRegName[arg2_reg_no];
    } else {
        // 操作数2不在寄存器中，需要加载到另一个临时寄存器
        printf("Debug: arg2 not in register, loading to temp register\n");
        int temp_reg = (s1 == PlatformArm64::regName[ARM64_TMP_REG_NO]) ? ARM64_TMP_REG_NO + 1 : ARM64_TMP_REG_NO;
        iloc.load_var(temp_reg, arg2);
        s2 = PlatformArm64::regName[temp_reg];
    }

    printf("Debug: Generating %s %s, %s, %s\n",
           operator_name.c_str(),
           PlatformArm64::floatRegName[result_reg_no].c_str(),
           s1.c_str(),
           s2.c_str());

    // 生成浮点数运算指令
    iloc.inst(operator_name, PlatformArm64::floatRegName[result_reg_no], s1, s2);
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

/// @brief 浮点数加法指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_add_f(Instruction * inst)
{
    translate_two_operator_float(inst, "fadd");
}

/// @brief 浮点数减法指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_sub_f(Instruction * inst)
{
    translate_two_operator_float(inst, "fsub");
}

/// @brief 浮点数乘法指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_mul_f(Instruction * inst)
{
    translate_two_operator_float(inst, "fmul");
}

/// @brief 浮点数除法指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_div_f(Instruction * inst)
{
    translate_two_operator_float(inst, "fdiv");
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

    // 生成符合标准的标签格式
    std::string trueLabel = ".L" + iftrue->getIRName();
    std::string falseLabel = ".L" + iffalse->getIRName();
    printf("Debug: trueLabel = %s, falseLabel = %s\n", trueLabel.c_str(), falseLabel.c_str());

    if (cond_reg_no == -1) {
        // 条件操作数在栈上，需要先加载到临时寄存器
        printf("Debug: Condition operand in memory, loading to temp register\n");
        iloc.load_var(ARM64_TMP_REG_NO, cond);
        iloc.inst("cmp", PlatformArm64::regName[ARM64_TMP_REG_NO], "#0");
    } else {
        // 条件操作数在寄存器中
        iloc.inst("cmp", PlatformArm64::regName[cond_reg_no], "#0");
    }

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

    // 检查是否为浮点数比较
    Value * arg1_val = inst->getOperand(0);
    Value * arg2_val = inst->getOperand(1);
    bool isFloatComparison = arg1_val->getType()->isFloatType() || arg2_val->getType()->isFloatType();

    // 处理常量操作数和寄存器名称
    std::string arg1_str, arg2_str;

    if (isFloatComparison) {
        // 浮点数比较使用浮点寄存器名称
        arg1_str = PlatformArm64::floatRegName[arg1_reg_no];

        if (Instanceof(constFloat, ConstFloat *, arg2)) {
            // 浮点数常量需要先加载到寄存器
            arg2_str = PlatformArm64::floatRegName[arg2_reg_no];
        } else {
            arg2_str = PlatformArm64::floatRegName[arg2_reg_no];
        }
    } else {
        // 整数比较使用通用寄存器名称
        if (arg1_reg_no == -1) {
            // 第一个操作数在栈上，需要先加载到临时寄存器
            iloc.load_var(ARM64_TMP_REG_NO, arg1);
            arg1_str = PlatformArm64::regName[ARM64_TMP_REG_NO];
        } else {
            arg1_str = PlatformArm64::regName[arg1_reg_no];
        }

        if (Instanceof(constVal, ConstInt *, arg2)) {
            // 第二个操作数是常量
            arg2_str = "#" + std::to_string(constVal->getVal());
        } else if (arg2_reg_no == -1) {
            // 第二个操作数在栈上，需要先加载到临时寄存器
            int temp_reg =
                (arg1_str == PlatformArm64::regName[ARM64_TMP_REG_NO]) ? ARM64_TMP_REG_NO + 1 : ARM64_TMP_REG_NO;
            iloc.load_var(temp_reg, arg2);
            arg2_str = PlatformArm64::regName[temp_reg];
        } else {
            arg2_str = PlatformArm64::regName[arg2_reg_no];
        }
    }

    // 根据比较类型设置结果寄存器
    IRInstOperator op = inst->getOp();
    std::string condition;

    if (isFloatComparison) {
        // 浮点数比较使用fcmp指令
        iloc.inst("fcmp", arg1_str, arg2_str);

        switch (op) {
            case IRInstOperator::IRINST_OP_LT:
                condition = "mi"; // minus (less than for floats)
                break;
            case IRInstOperator::IRINST_OP_LE:
                condition = "ls"; // lower or same
                break;
            case IRInstOperator::IRINST_OP_GT:
                condition = "gt"; // greater than
                break;
            case IRInstOperator::IRINST_OP_GE:
                condition = "ge"; // greater or equal
                break;
            case IRInstOperator::IRINST_OP_EQ:
                condition = "eq"; // equal
                break;
            case IRInstOperator::IRINST_OP_NE:
                condition = "ne"; // not equal
                break;
            default:
                condition = "mi"; // 默认为小于
                break;
        }
    } else {
        // 整数比较使用cmp指令
        iloc.inst("cmp", arg1_str, arg2_str);

        switch (op) {
            case IRInstOperator::IRINST_OP_LT:
            case IRInstOperator::IRINST_OP_ICMP:
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
        // 检查arg1是否是getelementptr的结果，需要重新计算地址
        if (GetelementptrInstruction * gepResult = dynamic_cast<GetelementptrInstruction *>(arg1)) {
            printf("Debug: 加载gep指令的结果\n");

            // getelementptr的结果在内存中，使用内存地址信息
            printf("Debug: getelementptr结果在内存中\n");
            int32_t base_reg_id = -1;
            int64_t base_offset = -1;
            if (gepResult->getMemoryAddr(&base_reg_id, &base_offset)) {
                iloc.inst("ldr",
                          PlatformArm64::regName[result_regId],
                          "[" + PlatformArm64::regName[base_reg_id] + ",#" + std::to_string(base_offset) + "]");
            }
        }
        // 内存变量 => 寄存器
        else {
            iloc.load_var(result_regId, arg1);
        }
    }
}

/// @brief store指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_store(Instruction * inst)
{
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_regId = arg1->getRegId();

    // 检查是否是数组类型的存储
    if (arg1->getType()->isArrayType()) {

        // 对于数组类型的存储，生成memcpy指令
        // arg1是源数组（全局变量），arg2是目标数组（局部变量）

        // 1. 获取数组大小
        int arraySize = arg1->getType()->getSize();

        // 2. 获取源地址（全局变量或从全局变量加载的值）
        int src_reg = ARM64_TMP_REG_NO + 1; // 使用x11作为源地址寄存器
        std::string globalVarName;

        if (GlobalVariable * globalVar = dynamic_cast<GlobalVariable *>(arg1)) {
            // 直接是全局变量
            globalVarName = globalVar->getName();
        } else if (LoadInstruction * loadInst = dynamic_cast<LoadInstruction *>(arg1)) {
            // 是从全局变量加载的值，获取源全局变量
            Value * loadSource = loadInst->getOperand(0);
            if (GlobalVariable * globalVar = dynamic_cast<GlobalVariable *>(loadSource)) {
                globalVarName = globalVar->getName();
                printf("Debug: translate_store - source is load from global variable %s\n", globalVarName.c_str());
            }
        }

        // 加载全局变量地址到寄存器
        iloc.inst("adrp", PlatformArm64::regName[src_reg + 32], globalVarName);
        iloc.inst("add",
                  PlatformArm64::regName[src_reg + 32],
                  PlatformArm64::regName[src_reg + 32],
                  ":lo12:" + globalVarName);

        // 3. 获取目标地址（局部变量）
        int dest_reg = ARM64_TMP_REG_NO; // 使用x10作为目标地址寄存器
        int32_t dest_baseRegId = -1;
        int64_t dest_offset = -1;
        if (arg2->getMemoryAddr(&dest_baseRegId, &dest_offset)) {
            // 计算目标地址：sp + offset
            if (dest_offset == 0) {
                iloc.inst("mov", PlatformArm64::regName[dest_reg + 32], PlatformArm64::regName[dest_baseRegId + 32]);
            } else {
                iloc.load_imm(dest_reg, dest_offset);
                iloc.inst("add", PlatformArm64::regName[dest_reg + 32], "sp", PlatformArm64::regName[dest_reg + 32]);
            }
        }

        // 4. 逐字复制数组内容
        int words = arraySize / 4;
        for (int i = 0; i < words; i++) {
            int offset = i * 4;
            // 从源地址加载
            iloc.inst("ldr",
                      "w" + std::to_string(ARM64_TMP_REG_NO + 2),
                      "[" + PlatformArm64::regName[src_reg + 32] + ",#" + std::to_string(offset) + "]");
            // 存储到目标地址
            iloc.inst("str",
                      "w" + std::to_string(ARM64_TMP_REG_NO + 2),
                      "[" + PlatformArm64::regName[dest_reg + 32] + ",#" + std::to_string(offset) + "]");
        }

        return;
    }

    // 检查是否是常量
    if (ConstInt * constVal = dynamic_cast<ConstInt *>(arg1)) {
        if (constVal->getVal() == 0) {
            // 常量0使用零寄存器，更高效
            int32_t dest_baseRegId = -1;
            int64_t dest_offset = -1;
            if (arg2->getMemoryAddr(&dest_baseRegId, &dest_offset)) {
                //使用str wzr指令
                std::string s = "[" + PlatformArm64::regName[dest_baseRegId] + ",#" + std::to_string(dest_offset) + "]";
                iloc.inst("str", "wzr", s);
            }
        }
    } else if (arg1_regId != -1) {
        // 寄存器 => 内存
        printf("寄存器 => 内存\n");

        // 检查目标是否是getelementptr的结果，需要重新计算地址
        int32_t dest_baseRegId = -1;
        int64_t dest_offset = -1;
        if (arg2->getMemoryAddr(&dest_baseRegId, &dest_offset)) {
            printf("目标有内存地址信息\n");
            // 目标有内存地址信息，重新计算地址到临时寄存器
            int32_t temp_reg = ARM64_TMP_REG_NO;
            std::string temp_reg_name = PlatformArm64::regName[temp_reg];
            std::string base_reg_name = PlatformArm64::regName[dest_baseRegId];
            std::string src_reg_name = PlatformArm64::regName[arg1_regId];

            // 确保使用64位寄存器进行地址计算
            if (temp_reg_name[0] == 'w')
                temp_reg_name[0] = 'x';
            if (base_reg_name[0] == 'w')
                base_reg_name[0] = 'x';

            // 重新计算目标地址，检查立即数范围
            // if (dest_offset >= -512 && dest_offset <= 508)
            std::string s = "[" + PlatformArm64::regName[dest_baseRegId] + ",#" + std::to_string(dest_offset) + "]";
            iloc.inst("str", PlatformArm64::regName[arg1_regId], s);

        } else {
            // 使用原来的方法
            iloc.store_var(arg1_regId, arg2, ARM64_TMP_REG_NO);
        }
    }
}

/// @brief ret指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_ret(Instruction * inst)
{
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

                // 检查偏移量是否在ldr指令的有效范围内
                // 对于32位数据：有符号偏移-256到+255，或无符号偏移0到16380（4字节对齐）
                if ((off >= -256 && off <= 255) || (off >= 0 && off <= 16380 && (off % 4) == 0)) {
                    // 偏移量在有效范围内，直接使用ldr指令
                    std::string s = "[sp,#" + std::to_string(off) + "]";
                    iloc.inst("ldr", PlatformArm64::regName[0], s);
                } else {
                    // 偏移量超出范围，使用间接寻址
                    iloc.load_imm(ARM64_TMP_REG_NO, off);
                    iloc.inst("add",
                              PlatformArm64::regName[ARM64_TMP_REG_NO + 32],
                              "sp",
                              PlatformArm64::regName[ARM64_TMP_REG_NO + 32]);
                    iloc.inst("ldr",
                              PlatformArm64::regName[0],
                              "[" + PlatformArm64::regName[ARM64_TMP_REG_NO + 32] + "]");
                }
            }
        }
    }

    iloc.emitFunctionEpilogue(func);
}

/// @brief fptosi指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_fptosi(Instruction * inst)
{
    // 浮点数转有符号整数：fcvtzs
    Value * src = inst->getOperand(0);
    int src_reg_no = src->getRegId();
    int result_reg_no = inst->getRegId();

    // 使用fcvtzs指令：浮点数转有符号整数（向零舍入）
    iloc.inst("fcvtzs", PlatformArm64::regName[result_reg_no], PlatformArm64::floatRegName[src_reg_no]);
}

/// @brief sitofp指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_sitofp(Instruction * inst)
{
    // 有符号整数转浮点数：scvtf
    Value * src = inst->getOperand(0);
    int src_reg_no = src->getRegId();
    int result_reg_no = inst->getRegId();

    // 使用scvtf指令：有符号整数转浮点数
    iloc.inst("scvtf", PlatformArm64::floatRegName[result_reg_no], PlatformArm64::regName[src_reg_no]);
}

/// @brief getelementptr指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_gep(Instruction * inst)
{
    Value * basePtr = inst->getOperand(0); // 基址指针
                                           // 对于数组访问，我们需要计算偏移量
    Value * index = inst->getOperand(2);
    // 这里简化处理：如果基址在内存中，我们计算其地址
    int32_t base_reg_id;
    int64_t base_offset;
    if (ConstInt * constIdx = dynamic_cast<ConstInt *>(index)) {
        // 索引为常量
        if (GetelementptrInstruction * gepBase = dynamic_cast<GetelementptrInstruction *>(basePtr)) {
            // 源是另一个getelementptr的结果
            int32_t base_reg_id = -1;
            int64_t base_offset = -1;
            gepBase->getMemoryAddr(&base_reg_id, &base_offset);
            printf("gep源是另一个gep的结果\n");

            // 计算第二维的偏移
            int64_t idx = constIdx->getVal();
            int64_t element_size = 4; // 基本元素大小
                                      // 检查gep源的类型来确定元素大小
            Type * baseType = basePtr->getType();
            if (baseType->isArrayType()) {
                printf("计算数组地址时检测到内部元素类型为数组类型\n");
                const ArrayType * arrayType = static_cast<const ArrayType *>(baseType);
                const std::vector<int> & dimensions = arrayType->getDimensions();

                if (dimensions.size() > 1) {
                    // 多维数组：每个元素是一个子数组
                    // 计算子数组的大小
                    int sub_array_size = 1;
                    for (size_t i = 1; i < dimensions.size(); i++) {
                        sub_array_size *= dimensions[i];
                    }
                    element_size = sub_array_size * arrayType->getElementType()->getSize();
                } else {
                    // 一维数组：每个元素是基本类型
                    element_size = arrayType->getElementType()->getSize();
                }
            } else if (baseType->isPointerType()) {
                printf("计算数组地址时检测到内部元素类型为指针类型\n");
                const PointerType * ptrType = static_cast<const PointerType *>(baseType);
                const Type * pointeeType = ptrType->getPointeeType();
                if (pointeeType->isArrayType()) {
                    const ArrayType * arrayType = static_cast<const ArrayType *>(pointeeType);
                    element_size = arrayType->getElementType()->getSize();
                } else {
                    element_size = pointeeType->getSize();
                }
            }

            // 计算偏移并添加到基地址
            int64_t offset = idx * element_size;
            base_offset += offset;
            inst->setMemoryAddr(base_reg_id, base_offset);
            return;
        }

        else if (basePtr->getMemoryAddr(&base_reg_id, &base_offset)) {
            printf("gep源在栈上\n");
            // gep源在栈上，计算其地址

            // 偏移为常量的情况
            int64_t idx = constIdx->getVal();

            // 计算正确的元素大小
            int64_t element_size = 4; // 默认int/float类型，4字节

            // 检查gep源的类型来确定元素大小
            Type * baseType = basePtr->getType();
            if (baseType->isArrayType()) {
                printf("计算数组地址时检测到内部元素类型为数组类型\n");
                const ArrayType * arrayType = static_cast<const ArrayType *>(baseType);
                const std::vector<int> & dimensions = arrayType->getDimensions();

                if (dimensions.size() > 1) {
                    // 多维数组：每个元素是一个子数组
                    // 计算子数组的大小
                    int sub_array_size = 1;
                    for (size_t i = 1; i < dimensions.size(); i++) {
                        sub_array_size *= dimensions[i];
                    }
                    element_size = sub_array_size * arrayType->getElementType()->getSize();
                } else {
                    // 一维数组：每个元素是基本类型
                    element_size = arrayType->getElementType()->getSize();
                }
            } else if (baseType->isPointerType()) {
                printf("计算数组地址时检测到内部元素类型为指针类型\n");
                const PointerType * ptrType = static_cast<const PointerType *>(baseType);
                const Type * pointeeType = ptrType->getPointeeType();
                if (pointeeType->isArrayType()) {
                    const ArrayType * arrayType = static_cast<const ArrayType *>(pointeeType);
                    element_size = arrayType->getElementType()->getSize();
                } else {
                    element_size = pointeeType->getSize();
                }
            }

            int64_t element_offset = base_offset + (idx * element_size);

            // 只设置结果的内存地址信息，不生成地址计算指令
            // 地址计算将在load/store指令中进行
            inst->setMemoryAddr(base_reg_id, element_offset);
            return;

            // 默认情况：只设置内存地址信息，不生成地址计算指令
            // 地址计算将在load/store指令中进行
            inst->setMemoryAddr(base_reg_id, base_offset);
        } else if (auto * globalArr = dynamic_cast<GlobalVariable *>(basePtr)) {
            // gep源是全局数组
            printf("gep源是全局数组: %s\n", globalArr->getName().c_str());
            int res_reg_id = inst->getRegId();

            // 加载全局数组的基地址
            iloc.inst("adrp", PlatformArm64::regName[res_reg_id + 32], globalArr->getName());
            iloc.inst("add",
                      PlatformArm64::regName[res_reg_id + 32],
                      PlatformArm64::regName[res_reg_id + 32],
                      ":lo12:" + globalArr->getName());

            // 计算索引偏移
            int64_t idx = constIdx->getVal();
            int64_t element_size = 4; // 默认元素大小

            // 检查全局数组的类型来确定正确的元素大小
            Type * globalType = globalArr->getStorageType() ? globalArr->getStorageType() : globalArr->getType();
            if (globalType->isArrayType()) {
                const ArrayType * arrayType = static_cast<const ArrayType *>(globalType);
                Type * elementType = arrayType->getElementType();
                if (elementType->isArrayType()) {
                    // 多维数组：第一个gep计算行偏移，元素大小是整行的大小
                    element_size = elementType->getSize();
                    printf("Debug: 多维数组第一个gep，行大小: %ld 字节\n", element_size);
                } else {
                    // 一维数组：元素大小是基本类型大小
                    element_size = elementType->getSize();
                }
            }

            int64_t off = idx * element_size;
            inst->setMemoryAddr(res_reg_id + 32, off);
        }
    } else {
        // 索引为变量
        printf("检测到变量索引: %s\n", index->getName().c_str());

        if (basePtr->getType()->isArrayType()) {
            // 检测到源为数组类型
            printf("源为数组类型\n");
            int index_reg_id = index->getRegId();
            const ArrayType * arrayType = static_cast<const ArrayType *>(basePtr->getType());
            const std::vector<int> & dimensions = arrayType->getDimensions();
            // 对于数组,获取最外s维
            int size = 1;
            for (int i = 1; i < dimensions.size(); i++) {
                size *= dimensions[i];
            }
            size *= arrayType->getElementType()->getSize();
            int lsl = 0;
            for (; size > 1; size >>= 1) {
                lsl++;
            }

            std::string op = "add";
            std::string s = PlatformArm64::regName[basePtr->getRegId() + 32] + "," +
                            PlatformArm64::regName[index_reg_id + 32] + ",lsl #" + std::to_string(lsl);
            if (dimensions.size() > 1) {
                //多维数组,add res_reg, base_ptr(全局/栈), 变量, lsl #最外围大小
            } else {
                //一维数组,ldr res_reg, [base_ptr(全局/栈/gep), 变量, lsl #最外围大小]
                op = "ldr";
                s = "[" + s + "]";
            }
            iloc.inst(op, PlatformArm64::regName[inst->getRegId() + 32], s);
        } else if (auto * globalArr = dynamic_cast<GlobalVariable *>(basePtr)) {
            // 处理全局数组变量的变量索引情况
            printf("gep源是全局数组(变量索引): %s\n", globalArr->getName().c_str());
            int res_reg_id = inst->getRegId();
            int index_reg_id = index->getRegId();

            // 加载全局数组的基地址
            iloc.inst("adrp", PlatformArm64::regName[res_reg_id + 32], globalArr->getName());
            iloc.inst("add",
                      PlatformArm64::regName[res_reg_id + 32],
                      PlatformArm64::regName[res_reg_id + 32],
                      ":lo12:" + globalArr->getName());

            // 计算变量索引的偏移
            int64_t element_size = 4; // 默认元素大小

            // 检查全局数组的类型来确定正确的元素大小
            Type * globalType = globalArr->getStorageType() ? globalArr->getStorageType() : globalArr->getType();
            if (globalType->isArrayType()) {
                const ArrayType * arrayType = static_cast<const ArrayType *>(globalType);
                Type * elementType = arrayType->getElementType();
                if (elementType->isArrayType()) {
                    // 多维数组：第一个gep计算行偏移，元素大小是整行的大小
                    element_size = elementType->getSize();
                    printf("Debug: 多维数组第一个gep(变量索引)，行大小: %ld 字节\n", element_size);
                } else {
                    // 一维数组：元素大小是基本类型大小
                    element_size = elementType->getSize();
                }
            }

            // 计算 element_size 对应的左移位数 (element_size = 2^shift)
            int shift = 0;
            int temp_size = element_size;
            while (temp_size > 1) {
                temp_size >>= 1;
                shift++;
            }

            // 生成地址计算指令: add res_reg, base_reg, index_reg, lsl #shift
            iloc.inst("add",
                      PlatformArm64::regName[res_reg_id + 32],
                      PlatformArm64::regName[res_reg_id + 32],
                      PlatformArm64::regName[index_reg_id + 32] + ",lsl #" + std::to_string(shift));
        } else if (GetelementptrInstruction * gepBase = dynamic_cast<GetelementptrInstruction *>(basePtr)) {
            // 处理源是另一个getelementptr结果的变量索引情况
            printf("gep源是另一个gep的结果(变量索引)\n");
            int32_t base_reg_id = -1;
            int64_t base_offset = -1;

            if (gepBase->getMemoryAddr(&base_reg_id, &base_offset)) {
                // 基址在内存中，计算变量索引的偏移
                int res_reg_id = inst->getRegId();
                int index_reg_id = index->getRegId();

                // 计算元素大小
                int64_t element_size = 4; // 默认元素大小
                Type * baseType = basePtr->getType();
                if (baseType->isPointerType()) {
                    const PointerType * ptrType = static_cast<const PointerType *>(baseType);
                    const Type * pointeeType = ptrType->getPointeeType();
                    if (pointeeType->isArrayType()) {
                        const ArrayType * arrayType = static_cast<const ArrayType *>(pointeeType);
                        element_size = arrayType->getElementType()->getSize();
                    } else {
                        element_size = pointeeType->getSize();
                    }
                }

                printf("Debug: 第二个gep(变量索引)，元素大小: %ld 字节\n", element_size);

                // 计算 element_size 对应的左移位数
                int shift = 0;
                int temp_size = element_size;
                while (temp_size > 1) {
                    temp_size >>= 1;
                    shift++;
                }

                // 获取第一个gep的寄存器信息
                int base_gep_reg = gepBase->getRegId();

                // 生成地址计算指令: add res_reg, base_reg, index_reg, lsl #shift
                if (base_gep_reg >= 0) {
                    // 第一个gep的结果已经在寄存器中，直接使用
                    printf("Debug: 使用第一个gep的寄存器结果: %s\n", PlatformArm64::regName[base_gep_reg + 32].c_str());
                    iloc.inst("add",
                              PlatformArm64::regName[res_reg_id + 32],
                              PlatformArm64::regName[base_gep_reg + 32],
                              PlatformArm64::regName[index_reg_id + 32] + ",lsl #" + std::to_string(shift));
                } else {
                    // 如果第一个gep的结果不在寄存器中，从内存加载
                    iloc.inst("add", PlatformArm64::regName[res_reg_id + 32], "sp", "#" + std::to_string(base_offset));
                    iloc.inst("add",
                              PlatformArm64::regName[res_reg_id + 32],
                              PlatformArm64::regName[res_reg_id + 32],
                              PlatformArm64::regName[index_reg_id + 32] + ",lsl #" + std::to_string(shift));
                }
            } else {
                printf("Warning: gep基址不在内存中，无法处理变量索引\n");
            }
        } else {
            printf("Warning: 未处理的变量索引情况，basePtr类型: %s\n", basePtr->getType()->toString().c_str());
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

    if (source_reg != -1) {
        // 源和目标都在寄存器中，目标由于寄存器分配，不可能没有分配到寄存器
        // 检查源是否有内存地址（alloca指令的结果或局部变量）
        int32_t base_reg_id;
        int64_t offset;
        bool hasMemAddr = source->getMemoryAddr(&base_reg_id, &offset);

        if (hasMemAddr) {
            // 针对Alloca分配的变量的情况
            // 源在栈上，需要计算地址而不是移动寄存器值
            // 源在栈上，计算栈地址
            std::string result_reg_name = PlatformArm64::regName[result_reg];
            std::string base_reg_name = PlatformArm64::regName[base_reg_id];

            // 确保使用64位寄存器
            if (result_reg_name[0] == 'w') {
                result_reg_name[0] = 'x';
            }
            if (base_reg_name[0] == 'w') {
                base_reg_name[0] = 'x';
            }

            iloc.inst("add", result_reg_name, base_reg_name, "#" + std::to_string(offset));

        } else if (result_reg != source_reg) {
            // 非alloca指令的普通寄存器移动
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
        // 源不在寄存器中，可能是全局变量
        if (auto globalVar = dynamic_cast<GlobalVariable *>(source)) {
            // 加载全局变量地址
            // adrp指令必须使用64位寄存器
            std::string result_reg_name = PlatformArm64::regName[result_reg];
            if (result_reg_name[0] == 'w') {
                result_reg_name[0] = 'x';
            }
            iloc.inst("adrp", result_reg_name, globalVar->getName());
            iloc.inst("add", result_reg_name, result_reg_name, ":lo12:" + globalVar->getName());
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

    // 如果地址不在寄存器中，需要先加载地址
    if (dest_reg == -1) {
        // 目标地址不在寄存器中，需要计算地址
        int32_t base_reg_id;
        int64_t base_offset;
        if (dest->getMemoryAddr(&base_reg_id, &base_offset)) {
            // 目标在栈上，使用临时寄存器计算地址
            dest_reg = ARM64_TMP_REG_NO;
            std::string dest_reg_name = PlatformArm64::regName[dest_reg];
            std::string base_reg_name = PlatformArm64::regName[base_reg_id];

            // 确保使用64位寄存器
            if (dest_reg_name[0] == 'w')
                dest_reg_name[0] = 'x';
            if (base_reg_name[0] == 'w')
                base_reg_name[0] = 'x';

            // 检查偏移量是否在add/sub指令的立即数范围内（0-4095）
            if (base_offset >= 0 && base_offset <= 4095) {
                iloc.inst("add", dest_reg_name, base_reg_name, "#" + std::to_string(base_offset));
            } else {
                // 偏移量超出范围，使用临时寄存器
                iloc.load_imm(ARM64_TMP_REG_NO + 1, base_offset); // 使用另一个临时寄存器
                iloc.inst("add", dest_reg_name, base_reg_name, PlatformArm64::regName[ARM64_TMP_REG_NO + 1 + 32]);
            }
        }
    }

    if (src_reg == -1) {
        // 源地址不在寄存器中，检查是否是全局变量
        if (auto globalVar = dynamic_cast<GlobalVariable *>(src)) {
            // 源是全局变量，使用另一个临时寄存器
            src_reg = (dest_reg == ARM64_TMP_REG_NO) ? ARM64_TMP_REG_NO + 1 : ARM64_TMP_REG_NO;
            std::string src_reg_name = PlatformArm64::regName[src_reg];
            if (src_reg_name[0] == 'w')
                src_reg_name[0] = 'x';

            // 加载全局变量地址
            iloc.inst("adrp", src_reg_name, globalVar->getName());
            iloc.inst("add", src_reg_name, src_reg_name, ":lo12:" + globalVar->getName());
        }
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

            // 选择一个不与dest_reg和src_reg冲突的临时寄存器
            int temp_reg = ARM64_TMP_REG_NO; // 默认使用w10
            if (temp_reg == dest_reg || temp_reg == src_reg) {
                temp_reg = ARM64_TMP_REG_NO + 1; // 使用w11
                if (temp_reg == dest_reg || temp_reg == src_reg) {
                    temp_reg = ARM64_TMP_REG_NO + 2; // 使用w12
                }
            }
            std::string temp_reg_name = PlatformArm64::regName[temp_reg];

            // 从源地址加载数据
            iloc.inst("ldr", temp_reg_name, "[" + src_reg_name + ", #" + std::to_string(i * 4) + "]");
            // 存储到目标地址
            iloc.inst("str", temp_reg_name, "[" + dest_reg_name + ", #" + std::to_string(i * 4) + "]");
        }
    }
}

/// @brief memset指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_memset(Instruction * inst)
{
    // memset指令用于内存设置（通常是清零）
    // 参数：dest, value, size, volatile
    Value * dest = inst->getOperand(0);
    Value * value = inst->getOperand(1);
    Value * size = inst->getOperand(2);

    // 获取目标地址的寄存器
    int32_t dest_reg = dest->getRegId();

    // 如果地址不在寄存器中，需要先加载地址
    if (dest_reg == -1) {
        // 目标地址不在寄存器中，需要计算地址
        int32_t base_reg_id;
        int64_t base_offset;
        if (dest->getMemoryAddr(&base_reg_id, &base_offset)) {
            // 目标在栈上，使用临时寄存器计算地址
            dest_reg = ARM64_TMP_REG_NO;
            std::string dest_reg_name = PlatformArm64::regName[dest_reg];
            std::string base_reg_name = PlatformArm64::regName[base_reg_id];

            // 确保使用64位寄存器
            if (dest_reg_name[0] == 'w')
                dest_reg_name[0] = 'x';
            if (base_reg_name[0] == 'w')
                base_reg_name[0] = 'x';

            iloc.inst("add", dest_reg_name, base_reg_name, "#" + std::to_string(base_offset));
        }
    }

    // 检查设置的值（通常是0）
    ConstInt * constValue = dynamic_cast<ConstInt *>(value);
    if (!constValue) {
        printf("Warning: memset with non-constant value not implemented\n");
        return;
    }

    int setValue = constValue->getVal();
    if (setValue != 0) {
        printf("Warning: memset with non-zero value not implemented\n");
        return;
    }

    // 获取设置大小
    if (auto constSize = dynamic_cast<ConstInt *>(size)) {
        int setSize = constSize->getVal();
        int wordCount = (setSize + 3) / 4; // 向上取整到字边界

        // 使用循环设置数据为零
        std::string dest_reg_name = PlatformArm64::regName[dest_reg];
        if (dest_reg_name[0] == 'w') {
            dest_reg_name[0] = 'x';
        }

        for (int i = 0; i < wordCount; i++) {
            // 存储零到目标地址
            iloc.inst("str", "wzr", "[" + dest_reg_name + ", #" + std::to_string(i * 4) + "]");
        }
    }
}

/// @brief sext指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_sext(Instruction * inst)
{
    Value * src = inst->getOperand(0);

    // 获取源类型和目标类型
    Type * srcType = src->getType();
    Type * destType = inst->getType();

    // 获取源寄存器和目标寄存器
    int32_t src_reg_no = src->getRegId();
    int32_t dest_reg_no = inst->getRegId();

    // 如果源值不在寄存器中，先加载到临时寄存器
    if (src_reg_no == -1) {
        iloc.load_var(ARM64_TMP_REG_NO, src);
        src_reg_no = ARM64_TMP_REG_NO;
    }

    // 获取源类型和目标类型的位宽
    int32_t srcBitWidth = 32;  // 默认32位
    int32_t destBitWidth = 32; // 默认32位

    if (Instanceof(srcIntType, IntegerType *, srcType)) {
        srcBitWidth = srcIntType->getBitWidth();
    }
    if (Instanceof(destIntType, IntegerType *, destType)) {
        destBitWidth = destIntType->getBitWidth();
    }

    // 根据源类型和目标类型选择合适的符号扩展指令
    std::string src_reg_name = PlatformArm64::regName[src_reg_no];
    std::string dest_reg_name = PlatformArm64::regName[dest_reg_no];

    if (srcBitWidth == 32 && destBitWidth == 64) {
        // i32 -> i64: 使用sxtw指令（符号扩展字到双字）
        // 需要使用64位寄存器名
        if (dest_reg_no < 32) {
            dest_reg_name = "x" + std::to_string(dest_reg_no);
        }
        iloc.inst("sxtw", dest_reg_name, src_reg_name);
    } else {
        printf("sext暂时不支持这种转换\n");
    }
}

/// @brief zext指令翻译成ARM64汇编
/// @param inst IR指令
void InstSelectorArm64::translate_zext(Instruction * inst)
{
    Value * src = inst->getOperand(0);

    // 获取源类型和目标类型
    Type * srcType = src->getType();
    Type * destType = inst->getType();

    // 获取源寄存器和目标寄存器
    int32_t src_reg_no = src->getRegId();
    int32_t dest_reg_no = inst->getRegId();

    // 如果源值不在寄存器中，先加载到临时寄存器
    if (src_reg_no == -1) {
        iloc.load_var(ARM64_TMP_REG_NO, src);
        src_reg_no = ARM64_TMP_REG_NO;
    }

    // 获取源类型和目标类型的位宽
    int32_t srcBitWidth = 32;  // 默认32位
    int32_t destBitWidth = 32; // 默认32位

    if (Instanceof(srcIntType, IntegerType *, srcType)) {
        srcBitWidth = srcIntType->getBitWidth();
    }
    if (Instanceof(destIntType, IntegerType *, destType)) {
        destBitWidth = destIntType->getBitWidth();
    }

    // 根据源类型和目标类型选择合适的零扩展指令
    std::string src_reg_name = PlatformArm64::regName[src_reg_no];
    std::string dest_reg_name = PlatformArm64::regName[dest_reg_no];

    printf("Debug: zext from %d bits to %d bits\n", srcBitWidth, destBitWidth);
    printf("Debug: src_reg_no=%d, dest_reg_no=%d\n", src_reg_no, dest_reg_no);

    if (srcBitWidth == 8 && destBitWidth == 32) {
        // i8 -> i32: 使用uxtb指令（零扩展字节到字）
        printf("Debug: Generating uxtb instruction\n");
        iloc.inst("uxtb", dest_reg_name, src_reg_name);
    } else if (srcBitWidth == 16 && destBitWidth == 32) {
        // i16 -> i32: 使用uxth指令（零扩展半字到字）
        printf("Debug: Generating uxth instruction\n");
        iloc.inst("uxth", dest_reg_name, src_reg_name);
    } else if (srcBitWidth == 1 && destBitWidth == 32) {
        // i1 -> i32: 布尔值零扩展到32位整数
        printf("Debug: Generating mov instruction for i1->i32 (zext)\n");
        iloc.inst("mov", dest_reg_name, src_reg_name);
    } else if (srcBitWidth == 32 && destBitWidth == 64) {
        // i32 -> i64: 在ARM64中，32位操作会自动零扩展到64位
        // 使用mov指令将32位值移动到64位寄存器，高32位自动清零
        if (dest_reg_no < 32) {
            dest_reg_name = "x" + std::to_string(dest_reg_no);
        }
        // 使用32位寄存器名作为源，会自动零扩展
        printf("Debug: Generating mov instruction for i32->i64 (zext)\n");
        iloc.inst("mov", dest_reg_name, src_reg_name);
    } else if (srcBitWidth == 8 && destBitWidth == 64) {
        // i8 -> i64: 零扩展字节到64位
        if (dest_reg_no < 32) {
            dest_reg_name = "x" + std::to_string(dest_reg_no);
        }
        printf("Debug: Generating uxtb instruction for i8->i64\n");
        iloc.inst("uxtb", dest_reg_name, src_reg_name);
    } else if (srcBitWidth == 16 && destBitWidth == 64) {
        // i16 -> i64: 零扩展半字到64位
        if (dest_reg_no < 32) {
            dest_reg_name = "x" + std::to_string(dest_reg_no);
        }
        printf("Debug: Generating uxth instruction for i16->i64\n");
        iloc.inst("uxth", dest_reg_name, src_reg_name);
    } else {
        // 其他情况，使用mov指令（如果位宽相同）
        printf("Debug: Generating mov instruction (fallback) for %d->%d\n", srcBitWidth, destBitWidth);
        iloc.inst("mov", dest_reg_name, src_reg_name);
    }
}