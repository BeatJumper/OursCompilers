///
/// @file ILocArm64.cpp
/// @brief 指令序列管理的实现，ILOC的全称为Intermediate Language for Optimizing Compilers
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
#include <cstdio>
#include <string>

#include "ILocArm64.h"
#include "Common.h"
#include "Function.h"
#include "PlatformArm64.h"
#include "Module.h"

ArmInst::ArmInst(std::string _opcode,
                 std::string _result,
                 std::string _arg1,
                 std::string _arg2,
                 std::string _cond,
                 std::string _addition)
    : opcode(_opcode), cond(_cond), result(_result), arg1(_arg1), arg2(_arg2), addition(_addition), dead(false)
{}

/*
    指令内容替换
*/
void ArmInst::replace(std::string _opcode,
                      std::string _result,
                      std::string _arg1,
                      std::string _arg2,
                      std::string _cond,
                      std::string _addition)
{
    opcode = _opcode;
    result = _result;
    arg1 = _arg1;
    arg2 = _arg2;
    cond = _cond;
    addition = _addition;

#if 0
    // 空操作，则设置为dead
    if (op == "") {
        dead = true;
    }
#endif
}

/*
    设置为无效指令
*/
void ArmInst::setDead()
{
    dead = true;
}

/*
    输出函数
*/
std::string ArmInst::outPut()
{
    // 无用代码，什么都不输出
    if (dead) {
        return "";
    }

    // 占位指令,可能需要输出一个空操作，看是否支持 FIXME
    if (opcode.empty()) {
        return "";
    }

    std::string ret = opcode;

    if (!cond.empty()) {
        ret += cond;
    }

    // 结果输出
    if (!result.empty()) {
        if (result == ":") {
            ret += result;
        } else {
            ret += " " + result;
        }
    }

    // 第一元参数输出
    if (!arg1.empty()) {
        ret += "," + arg1;
    }

    // 第二元参数输出
    if (!arg2.empty()) {
        ret += "," + arg2;
    }

    // 其他附加信息输出
    if (!addition.empty()) {
        ret += "," + addition;
    }

    return ret;
}

#define emit(...) code.push_back(new ArmInst(__VA_ARGS__))

/// @brief 构造函数
/// @param _module 符号表
ILocArm64::ILocArm64(Module * _module)
{
    this->module = _module;
}

/// @brief 析构函数
ILocArm64::~ILocArm64()
{
    std::list<ArmInst *>::iterator pIter;

    for (pIter = code.begin(); pIter != code.end(); ++pIter) {
        delete (*pIter);
    }
}

/// @brief 删除无用的Label指令
void ILocArm64::deleteUsedLabel()
{
    std::list<ArmInst *> labelInsts;
    for (ArmInst * arm: code) {
        if ((!arm->dead) && (arm->opcode[0] == '.') && (arm->result == ":")) {
            labelInsts.push_back(arm);
        }
    }

    for (ArmInst * labelArm: labelInsts) {
        bool labelUsed = false;

        for (ArmInst * arm: code) {
            // TODO 转移语句的指令标识符根据定义修改判断
			// 判断当前指令是否为未被标记为无效的转移指令，且转移目标为当前标签指令。
			// 在 ARM 汇编中，转移指令的操作码通常以 b 开头。
            if ((!arm->dead) && (arm->opcode[0] == 'b') && (arm->result == labelArm->opcode)) {
                labelUsed = true;
                break;
            }
        }

        if (!labelUsed) {
            labelArm->setDead();
        }
    }
}

/// @brief 输出汇编
/// @param file 输出的文件指针
/// @param outputEmpty 是否输出空语句
void ILocArm64::outPut(FILE * file, bool outputEmpty)
{
    for (auto arm: code) {

        std::string s = arm->outPut();

        if (arm->result == ":") {
            // Label指令，不需要Tab输出
            fprintf(file, "%s\n", s.c_str());
            continue;
        }

        if (!s.empty()) {
            fprintf(file, "\t%s\n", s.c_str());
        } else if ((outputEmpty)) {
            fprintf(file, "\n");
        }
    }
}

/// @brief 获取当前的代码序列
/// @return 代码序列
std::list<ArmInst *> & ILocArm64::getCode()
{
    return code;
}

/**
 * 数字变字符串，若flag为真，则变为立即数寻址（加#）
 */
std::string ILocArm64::toStr(int64_t num, bool flag)
{
    std::string ret;

    if (flag) {
        ret = "#";
    }

    ret += std::to_string(num);

    return ret;
}

/*
    产生标签
*/
void ILocArm64::label(std::string name)
{
    // .L1:
    emit(name, ":");
}

/// @brief 0个源操作数指令
/// @param op 操作码
/// @param rs 操作数
void ILocArm64::inst(std::string op, std::string rs)
{
    emit(op, rs);
}

/// @brief 一个操作数指令
/// @param op 操作码
/// @param rs 操作数
/// @param arg1 源操作数
void ILocArm64::inst(std::string op, std::string rs, std::string arg1)
{
    emit(op, rs, arg1);
}

/// @brief 一个操作数指令
/// @param op 操作码
/// @param rs 操作数
/// @param arg1 源操作数
/// @param arg2 源操作数
void ILocArm64::inst(std::string op, std::string rs, std::string arg1, std::string arg2)
{
    emit(op, rs, arg1, arg2);
}

///
/// @brief 注释指令，不包含分号
///
void ILocArm64::comment(std::string str)
{
    emit("@", str);
}

/*
    加载立即数 ldr r0,=#100
*/
void ILocArm64::load_imm(int rs_reg_no, int64_t constant)
{
    // movz: 将 16 位立即数零扩展到 64 位寄存器
    // movk: 将 16 位立即数插入到寄存器的指定 16 位位置而不影响其他位

    // 处理低 16 位
    emit("movz", PlatformArm64::regName[rs_reg_no], "#" + std::to_string(constant & 0xFFFF), "lsl #0");

    // 处理接下来的 16 位
    if ((constant >> 16) & 0xFFFF) {
        emit("movk", PlatformArm64::regName[rs_reg_no], "#" + std::to_string((constant >> 16) & 0xFFFF), "lsl #16");
    }

    // 处理再接下来的 16 位
    if ((constant >> 32) & 0xFFFF) {
        emit("movk", PlatformArm64::regName[rs_reg_no], "#" + std::to_string((constant >> 32) & 0xFFFF), "lsl #32");
    }

    // 处理最高的 16 位
    if ((constant >> 48) & 0xFFFF) {
        emit("movk", PlatformArm64::regName[rs_reg_no], "#" + std::to_string((constant >> 48) & 0xFFFF), "lsl #48");
    }
}

/// @brief 加载符号值 ldr r0,=g ldr r0,=.L1
/// @param rs_reg_no 结果寄存器编号
/// @param name 符号名
void ILocArm64::load_symbol(int rs_reg_no, std::string name)
{
    // adrp 指令加载符号所在页的基地址到指定寄存器
    emit("adrp", PlatformArm64::regName[rs_reg_no], name);

    // add 指令将符号在页内的偏移量加到基地址上
    // :lo12: 表示取符号地址的低 12 位作为偏移量
    emit("add", PlatformArm64::regName[rs_reg_no], PlatformArm64::regName[rs_reg_no], ":" + name + ":lo12");
}

/// @brief 基址寻址 ldr r0,[fp,#100]
/// @param rsReg 结果寄存器
/// @param base_reg_no 基址寄存器
/// @param offset 偏移
void ILocArm64::load_base(int rs_reg_no, int base_reg_no, int64_t offset)
{
    std::string rsReg = PlatformArm64::regName[rs_reg_no];
    std::string base = PlatformArm64::regName[base_reg_no];

    if (PlatformArm64::isDisp(offset)) {
        // 有效的偏移常量
        if (offset) {
            // [fp,#-16] [fp]
            base += "," + toStr(offset);
        }
    } else {

        // ldr r8,=-4096
        load_imm(rs_reg_no, offset);

        // fp,r8
        base += "," + rsReg;
    }

    // 内存寻址
    base = "[" + base + "]";

    // ldr x8,[fp,#-8]
    // ldr x8,[fp,x8]
    emit("ldr", rsReg, base);
}

/// @brief 基址寻址 str x0,[fp,#100]
/// @param srcReg 源寄存器
/// @param base_reg_no 基址寄存器
/// @param disp 偏移
/// @param tmp_reg_no 可能需要临时寄存器编号
void ILocArm64::store_base(int src_reg_no, int base_reg_no, int64_t disp, int tmp_reg_no)
{
    std::string base = PlatformArm64::regName[base_reg_no];

    if (PlatformArm64::isDisp(disp)) {
        // 有效的偏移常量

        // 若disp为0，则直接采用基址，否则采用基址+偏移
        // [fp,#-16] [fp]
        if (disp) {
            base += "," + toStr(disp);
        }
    } else {
        // 先把立即数赋值给指定的寄存器tmpReg，然后采用基址+寄存器的方式进行

        // ldr x9,=-4096
        load_imm(tmp_reg_no, disp);

        // fp,x9
        base += "," + PlatformArm64::regName[tmp_reg_no];
    }

    // 内存间接寻址
    base = "[" + base + "]";

    // str x8,[fp,#-8]
    // str x8,[fp,x9]
    emit("str", PlatformArm64::regName[src_reg_no], base);
}

/// @brief 寄存器Mov操作
/// @param rs_reg_no 结果寄存器
/// @param src_reg_no 源寄存器
void ILocArm64::mov_reg(int rs_reg_no, int src_reg_no)
{
    emit("mov", PlatformArm64::regName[rs_reg_no], PlatformArm64::regName[src_reg_no]);
}

/// @brief 加载变量到寄存器，保证将变量放到reg中
/// @param rs_reg_no 结果寄存器
/// @param src_var 源操作数
void ILocArm64::load_var(int rs_reg_no, Value * src_var)
{

    if (Instanceof(constVal, ConstInt *, src_var)) {
        // 整型常量

        // TODO 目前只考虑整数类型 100
        // ldr x8,#100
        load_imm(rs_reg_no, constVal->getVal());
    } else if (src_var->getRegId() != -1) {

        // 源操作数为寄存器变量
        int32_t src_regId = src_var->getRegId();

        if (src_regId != rs_reg_no) {

            // mov x8,x2 | 这里有优化空间——消除x8
            emit("mov", PlatformArm64::regName[rs_reg_no], PlatformArm64::regName[src_regId]);
        }
    } else if (Instanceof(globalVar, GlobalVariable *, src_var)) {
        // 全局变量

        // 读取全局变量的地址
        // adrp x8, symbol@PAGE
        // add x8, x8, symbol@PAGEOFF
        load_symbol(rs_reg_no, globalVar->getName());

        // ldr x8, [x8]
        emit("ldr", PlatformArm64::regName[rs_reg_no], "[" + PlatformArm64::regName[rs_reg_no] + "]");

    } else {

        // 栈+偏移的寻址方式

        // 栈帧偏移
        int32_t var_baseRegId = -1;
        int64_t var_offset = -1;

        bool result = src_var->getMemoryAddr(&var_baseRegId, &var_offset);
        if (!result) {
            minic_log(LOG_ERROR, "BUG");
        }

        // 对于栈内分配的局部数组，可直接在栈指针上进行移动与运算
        // 但对于形参，其保存的是调用函数栈的数组的地址，需要读取出来

        // ldr x8,[sp,#8]
        load_base(rs_reg_no, var_baseRegId, var_offset);
    }
}

/// @brief 加载变量地址到寄存器
/// @param rs_reg_no
/// @param var
void ILocArm64::lea_var(int rs_reg_no, Value * var)
{
    // 被加载的变量肯定不是常量！
    // 被加载的变量肯定不是寄存器变量！

    // 目前只考虑局部变量

    // 栈帧偏移
    int32_t var_baseRegId = -1;
    int64_t var_offset = -1;

    bool result = var->getMemoryAddr(&var_baseRegId, &var_offset);
    if (!result) {
        minic_log(LOG_ERROR, "BUG");
    }

    // lea x8, [fp,#8]
    leaStack(rs_reg_no, var_baseRegId, var_offset);
}

/// @brief 保存寄存器到变量，保证将计算结果（x8）保存到变量
/// @param src_reg_no 源寄存器
/// @param dest_var  变量
/// @param tmp_reg_no 第三方寄存器
void ILocArm64::store_var(int src_reg_no, Value * dest_var, int tmp_reg_no)
{
    // 被保存目标变量肯定不是常量

    if (dest_var->getRegId() != -1) {

        // 寄存器变量

        // -1表示非寄存器，其他表示寄存器的索引值
        int dest_reg_id = dest_var->getRegId();

        // 寄存器不一样才需要mov操作
        if (src_reg_no != dest_reg_id) {

            // mov x2,x8 | 这里有优化空间——消除r8
            emit("mov", PlatformArm64::regName[dest_reg_id], PlatformArm64::regName[src_reg_no]);
        }

    } else if (Instanceof(globalVar, GlobalVariable *, dest_var)) {
        // 全局变量

        // 读取符号的地址到寄存器x10
        load_symbol(tmp_reg_no, globalVar->getName());

        // str x8, [x10]
        emit("str", PlatformArm64::regName[src_reg_no], "[" + PlatformArm64::regName[tmp_reg_no] + "]");

    } else {

        // 对于局部变量，则直接从栈基址+偏移寻址

        // TODO 目前只考虑局部变量

        // 栈帧偏移
        int32_t dest_baseRegId = -1;
        int64_t dest_offset = -1;

        bool result = dest_var->getMemoryAddr(&dest_baseRegId, &dest_offset);
        if (!result) {
            minic_log(LOG_ERROR, "BUG");
        }

        // str x8,[x9]
        // str x8, [fp, # - 8]
        store_base(src_reg_no, dest_baseRegId, dest_offset, tmp_reg_no);
    }
}

/// @brief 加载栈内变量地址
/// @param rsReg 结果寄存器号
/// @param base_reg_no 基址寄存器
/// @param off 偏移
void ILocArm64::leaStack(int rs_reg_no, int base_reg_no, int64_t off)
{
    std::string rs_reg_name = PlatformArm64::regName[rs_reg_no];
    std::string base_reg_name = PlatformArm64::regName[base_reg_no];

    if (PlatformArm64::constExpr(off))
        // add x8,fp,#-16
        emit("add", rs_reg_name, base_reg_name, toStr(off));
    else {
        // ldr x8,=-257
        load_imm(rs_reg_no, off);

        // add x8,fp,r8
        emit("add", rs_reg_name, base_reg_name, rs_reg_name);
    }
}

/// @brief 函数内栈内空间分配（局部变量、形参变量、函数参数传值，或不能寄存器分配的临时变量等）
/// @param func 函数
/// @param tmp_reg_No
void ILocArm64::allocStack(Function * func, int tmp_reg_no)
{
    // 超过八个的函数调用参数个数，多于8个，则需要栈传值
    int64_t funcCallArgCnt = int64_t(std::max(func->getMaxFuncCallArgCnt() - 8, 0));

    // 计算栈帧大小
    int64_t off = func->getMaxDep();
    off += funcCallArgCnt * 4;

    // 不需要在栈内额外分配空间，则什么都不做
    if (0 == off)
        return;

    if (PlatformArm64::constExpr(off)) {
        // sub sp,sp,#16
        emit("sub", "sp", "sp", toStr(off));
    } else {
        // ldr r8,=257
        load_imm(tmp_reg_no, off);

        // sub sp,sp,r8
        emit("sub", "sp", "sp", PlatformArm64::regName[tmp_reg_no]);
    }

    // 函数调用通过栈传递的基址寄存器设置
    inst("add", PlatformArm64::regName[ARM64_FP_REG_NO], "sp", toStr(funcCallArgCnt * 4));
}

/// @brief 调用函数fun
/// @param fun
void ILocArm64::call_fun(std::string name)
{
    // 函数返回值在r0,不需要保护
    emit("bl", name);
}

/// @brief NOP操作
void ILocArm64::nop()
{
    // FIXME 无操作符，要确认是否用nop指令
    emit("nop");
}

///
/// @brief 无条件跳转指令
/// @param label 目标Label名称
///
void ILocArm64::jump(std::string label)
{
    emit("b", label);
}
