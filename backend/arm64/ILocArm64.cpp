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
#include <iostream>
#include <cstring>

#include "ILocArm64.h"
#include "Common.h"
#include "Function.h"
#include "PlatformArm64.h"
#include "Module.h"
#include "ConstFloat.h"

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

/// @brief 删除无用的指令
void ILocArm64::deleteUsedLabel()
{
    std::list<ArmInst *> Insts;
    for (ArmInst * arm: code) {
        if ((!arm->dead) && (arm->opcode[0] == '.') && (arm->opcode.find(".L") == 0) && // 只处理.L开头的标签
            (arm->result == ":")) {
            Insts.push_back(arm);
        }
    }

    for (ArmInst * Arm: Insts) {
        bool labelUsed = false;

        for (ArmInst * arm: code) {
            // 检查所有分支指令（b, beq, bne等）
            if ((!arm->dead) && (arm->opcode.find("b") == 0) && // 所有b开头的指令
                (arm->result == Arm->opcode)) {
                labelUsed = true;
                break;
            }
        }

        if (!labelUsed) {
            Arm->setDead();
        }
    }
}

/// @brief 输出汇编
/// @param file 输出的文件指针
/// @param outputEmpty 是否输出空语句
void ILocArm64::outPut(FILE * file, bool outputEmpty)
{
    for (auto arm: code) {
        // 跳过无效指令（包括被删除的标签）
        if (arm->dead) {
            continue;
        }
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
    printf("循环完毕\n");
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
    emit("mov", PlatformArm64::regName[rs_reg_no], "#" + std::to_string(constant));
}

/*
    加载浮点数立即数
*/
void ILocArm64::load_float_imm(int rs_reg_no, float val)
{
    // 对于浮点数常量，ARM64通常需要通过内存加载
    // 这里我们使用一个简化的方法：将浮点数转换为整数位模式，然后移动到浮点寄存器

    // 将float转换为uint32_t的位模式
    uint32_t bits;
    std::memcpy(&bits, &val, sizeof(float));

    // 先将位模式加载到通用寄存器
    emit("mov", "w" + std::to_string(rs_reg_no + 32), "#" + std::to_string(bits));

    // 然后从通用寄存器移动到浮点寄存器
    emit("fmov", "s" + std::to_string(rs_reg_no), "w" + std::to_string(rs_reg_no + 32));
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
    emit("add", PlatformArm64::regName[rs_reg_no], PlatformArm64::regName[rs_reg_no], ":lo12:" + name);
}

/// @brief 基址寻址 ldr r0,[fp,#100]
/// @param rsReg 结果寄存器
/// @param base_reg_no 基址寄存器
/// @param offset 偏移
void ILocArm64::load_base(int rs_reg_no, int base_reg_no, int64_t offset)
{
    std::string rsReg = PlatformArm64::regName[rs_reg_no];
    std::string base = PlatformArm64::regName[base_reg_no];
    std::cout << "基址寻址中,结果寄存器" << rsReg << "\n";

    // 检查偏移量是否在ldr指令的有效范围内
    // 对于32位数据：有符号偏移-256到+255，或无符号偏移0到16380（4字节对齐）
    if ((offset >= -256 && offset <= 255) || (offset >= 0 && offset <= 16380 && (offset % 4) == 0)) {
        // 有效的偏移常量
        if (offset) {
            // [fp,#-16] [fp]
            base += "," + toStr(offset);
        }
    } else {
        // 偏移量超出范围，使用寄存器间接寻址
        // ldr r8,=large_offset
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

    // 检查偏移量是否在str指令的有效范围内
    // 对于32位数据：有符号偏移-256到+255，或无符号偏移0到16380（4字节对齐）
    if ((disp >= -256 && disp <= 255) || (disp >= 0 && disp <= 16380 && (disp % 4) == 0)) {
        // 有效的偏移常量

        // 若disp为0，则直接采用基址，否则采用基址+偏移
        // [fp,#-16] [fp]
        if (disp) {
            base += "," + toStr(disp);
        }
    } else {
        // 偏移量超出范围，使用寄存器间接寻址
        // 先把立即数赋值给指定的寄存器tmpReg，然后采用基址+寄存器的方式进行

        // ldr x9,=large_offset
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
    printf("Debug: load_var - rs_reg_no=%d, src_var=%p\n", rs_reg_no, src_var);

    if (src_var == nullptr) {
        printf("Error: load_var - src_var is null\n");
        return;
    }

    printf("Debug: load_var - src_var name=%s, IRName=%s\n", src_var->getName().c_str(), src_var->getIRName().c_str());

    if (Instanceof(constVal, ConstInt *, src_var)) {
        // 整型常量
        // mov w8,#100
        load_imm(rs_reg_no, constVal->getVal());
    } else if (Instanceof(constFloat, ConstFloat *, src_var)) {
        // 浮点数常量
        // 对于浮点数常量，需要将其加载到浮点寄存器
        load_float_imm(rs_reg_no, constFloat->getVal());
    } else if (src_var->getRegId() != -1) {

        // 源操作数为寄存器变量
        // 对于load指令，寄存器中存储的是地址，需要从地址加载数据
        int32_t src_regId = src_var->getRegId();

        // 确保使用64位寄存器进行地址访问
        std::string src_reg_name = PlatformArm64::regName[src_regId];
        std::string result_reg_name = PlatformArm64::regName[rs_reg_no];
        if (src_reg_name[0] == 'w') {
            src_reg_name[0] = 'x';
        }

        // ldr w8, [x2] - 从寄存器中的地址加载数据
        emit("ldr", result_reg_name, "[" + src_reg_name + "]");
    } else if (Instanceof(globalVar, GlobalVariable *, src_var)) {
        // 全局变量

        // 读取全局变量的地址
        // adrp x8, symbol@PAGE
        // add x8, x8, symbol@PAGEOFF
        load_symbol(rs_reg_no + 32, globalVar->getName());

        // ldr x8, [x8]
        emit("ldr", PlatformArm64::regName[rs_reg_no], "[" + PlatformArm64::regName[rs_reg_no + 32] + "]");

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
            printf("赋值，寄存器到寄存器\n");
            // mov x2,x8 | 这里有优化空间——消除x8
            emit("mov", PlatformArm64::regName[dest_reg_id], PlatformArm64::regName[src_reg_no]);
        }

    } else if (Instanceof(globalVar, GlobalVariable *, dest_var)) {
        // 全局变量

        // 读取符号的地址到寄存器x10
        load_symbol(tmp_reg_no + 32, globalVar->getName());

        // str x8, [x10]
        emit("str", PlatformArm64::regName[src_reg_no], "[" + PlatformArm64::regName[tmp_reg_no + 32] + "]");

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

    // 确保使用64位寄存器进行地址计算
    if (rs_reg_name[0] == 'w') {
        rs_reg_name[0] = 'x';
    }
    if (base_reg_name[0] == 'w') {
        base_reg_name[0] = 'x';
    }

    // 检查偏移量是否在add/sub指令的立即数范围内（0-4095）
    if (off >= 0 && off <= 4095) {
        // 正偏移量在有效范围内，使用add指令
        emit("add", rs_reg_name, base_reg_name, toStr(off));
    } else if (off < 0 && (-off) <= 4095) {
        // 负偏移量在有效范围内，使用sub指令
        emit("sub", rs_reg_name, base_reg_name, toStr(-off));
    } else {
        // 偏移量超出范围，使用临时寄存器
        load_imm(rs_reg_no, off);
        emit("add", rs_reg_name, base_reg_name, rs_reg_name);
    }
}

/// @brief 函数内栈内空间分配（局部变量、形参变量、函数参数传值，或不能寄存器分配的临时变量等）
/// @param func 函数
/// @param tmp_reg_No
void ILocArm64::allocStack(Function * func, int tmp_reg_no)
{
    // 重新计算栈帧大小，确保所有alloca指令的空间都被正确计算
    int64_t allocaSize = 0;
    int64_t localVarSize = 0;
    int64_t tempVarSize = 0;

    // 计算所有alloca指令的空间需求
    for (auto inst: func->getInterCode().getInsts()) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_ALLOCA) {
            Type * allocatedType = inst->getType();
            int64_t size = allocatedType ? allocatedType->getSize() : 8;
            size = (size + 7) & ~7; // 对齐到8字节
            allocaSize += size;
        }
    }

    // 计算局部变量的空间需求（非数组类型）
    for (auto & local: func->getVarValues()) {
        if (!local->getType()->isArrayType()) {
            localVarSize += local->getType()->getSize();
        }
    }

    // 计算临时变量的空间需求
    for (auto inst: func->getInterCode().getInsts()) {
        if (inst->hasResultValue() && inst->getOp() != IRInstOperator::IRINST_OP_ALLOCA && inst->getRegId() == -1) {
            int32_t size = inst->getType()->getSize();
            size += (4 - size % 4) % 4; // 对齐到4字节
            tempVarSize += size;
        }
    }

    // 计算总的栈帧大小
    int totalSize = allocaSize + localVarSize + tempVarSize;
    int protectedRegNum = 0;

    // 保存寄存器空间
    if (func->getExistFuncCall()) {
        protectedRegNum = func->getProtectedReg().size();
        totalSize += protectedRegNum * 8;
    }

    // 栈传参数空间(超过8个的参数)，先按4字节分配(int,float)
    int stackArgSize = std::max(func->getRealArgcount() - 8, 0) * 4;
    func->setExtraStackSize(stackArgSize);
    totalSize += stackArgSize;

    // 对齐到16字节边界(ARM64要求)
    totalSize = (totalSize + 15) & ~15;

    func->setStackFrameSize(totalSize);

    // 计算保存寄存器的偏移量
    int64_t saveOffset = totalSize - protectedRegNum * 8;
    std::string off;

    // 检查偏移量是否在stp/ldp指令的有效范围内（-512到+504，且必须8字节对齐）
    if (saveOffset >= -512 && saveOffset <= 504 && (saveOffset % 8) == 0) {
        // 直接使用立即数偏移
        off = "[sp, #" + std::to_string(saveOffset) + "]";
    } else {
        // 偏移量超出范围，使用寄存器间接寻址
        // 这种情况下我们需要在函数序言中处理，暂时使用占位符
        off = "[sp, #LARGE_OFFSET]";
    }

    // 局部变量空间 - 从alloca分配的空间之后开始分配
    // 首先找到alloca指令分配的最大偏移量
    int64_t maxAllocaOffset = 0;
    for (auto inst: func->getInterCode().getInsts()) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_ALLOCA) {
            int32_t base;
            int64_t offset;
            if (inst->getMemoryAddr(&base, &offset)) {
                Type * allocatedType = inst->getType();
                int64_t size = allocatedType ? allocatedType->getSize() : 8;
                size = (size + 7) & ~7; // 对齐到8字节
                maxAllocaOffset = std::max(maxAllocaOffset, offset + size);
            }
        }
    }

    // 从alloca空间之后开始分配局部变量
    int64_t localVarOffset = maxAllocaOffset;

    for (auto & local: func->getVarValues()) {
        // 检查这个变量是否是alloca指令的结果
        std::string localName = local->getName();

        bool isAllocaResult = false;

        // 检查是否已经通过alloca指令分配了内存
        for (auto inst: func->getInterCode().getInsts()) {
            if (inst->getOp() == IRInstOperator::IRINST_OP_ALLOCA) {
                if (inst->getOperandsNum() > 0) {
                    Value * allocaResult = inst->getOperand(0);
                    if (allocaResult == local) {
                        isAllocaResult = true;
                        break;
                    }
                }
            }
        }

        if (isAllocaResult) {
            continue;
        }

        // 对齐到4字节边界
        localVarOffset = (localVarOffset + 3) & ~3;
        local->setOffset(localVarOffset);

        localVarOffset += local->getType()->getSize();
    }

    // 重新设置临时变量的偏移量，确保在栈帧范围内
    // 临时变量从局部变量空间之后开始分配
    int64_t temp_offset = localVarOffset;

    for (auto inst: func->getInterCode().getInsts()) {
        if (inst->hasResultValue() && inst->getOp() != IRInstOperator::IRINST_OP_ALLOCA) {
            // 跳过alloca指令，因为它们的内存地址已经在stackAlloc中设置
            int32_t size = inst->getType()->getSize();
            // 按照4字节的大小整数倍分配
            size += (4 - size % 4) % 4;
            temp_offset -= size;
            // 确保偏移量不小于0
            if (temp_offset < 0) {
                temp_offset = 0;
            }

            // 检查是否已经有内存地址（可能是alloca指令的结果）
            int32_t existing_base;
            int64_t existing_offset;
            if (!inst->getMemoryAddr(&existing_base, &existing_offset)) {
                // 只有当指令还没有内存地址时才设置
                inst->setMemoryAddr(ARM64_SP_REG_NO, temp_offset);
            }
        }
    }

    // 检查栈分配大小是否超出sub指令的立即数范围（0-4095）
    if (totalSize <= 4095) {
        // 小栈帧，直接使用sub指令
        std::string s = "#" + std::to_string(totalSize);
        emit("sub", "sp", "sp", s);
    } else {
        // 大栈帧，使用临时寄存器
        load_imm(ARM64_TMP_REG_NO, totalSize);
        emit("sub", "sp", "sp", PlatformArm64::regName[ARM64_TMP_REG_NO + 32]);
    }

    if (func->getExistFuncCall()) {
        // 非叶子函数：保存 FP 和 LR
        int64_t saveOffset = totalSize - protectedRegNum * 8;

        if (saveOffset >= -512 && saveOffset <= 504 && (saveOffset % 8) == 0) {
            // 偏移量在有效范围内，直接使用stp指令
            std::string validOff = "[sp, #" + std::to_string(saveOffset) + "]";
            emit("stp", "x29", "x30", validOff);
        } else {
            // 偏移量超出范围，使用间接寻址
            // 先计算地址到临时寄存器
            load_imm(ARM64_TMP_REG_NO, saveOffset);
            emit("add",
                 PlatformArm64::regName[ARM64_TMP_REG_NO + 32],
                 "sp",
                 PlatformArm64::regName[ARM64_TMP_REG_NO + 32]);
            emit("stp", "x29", "x30", "[" + PlatformArm64::regName[ARM64_TMP_REG_NO + 32] + "]");
        }

        // 设置新帧指针
        if (saveOffset >= 0 && saveOffset <= 4095) {
            emit("add", "x29", "sp", "#" + std::to_string(saveOffset));
        } else {
            // 偏移量超出范围，使用临时寄存器
            load_imm(ARM64_TMP_REG_NO + 1, saveOffset); // 使用另一个临时寄存器避免冲突
            emit("add", "x29", "sp", PlatformArm64::regName[ARM64_TMP_REG_NO + 1 + 32]);
        }
    }
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

/// @brief 生成函数结尾(恢复栈帧)
void ILocArm64::emitFunctionEpilogue(Function * func)
{
    /*// 恢复保留的寄存器
    int offset = 16;
    for (auto it = func->getProtectedReg().rbegin(); it != func->getProtectedReg().rend(); ++it) {
        emit("ldr", PlatformArm64::regName[*it], "[x29, #" + std::to_string(offset) + "]");
        offset -= 8;
    }*/

    int size = func->getStackFrameSize();
    int protectedRegNum = 0;
    if (func->getExistFuncCall()) {
        protectedRegNum = func->getProtectedReg().size();
    }

    // 恢复FP和LR
    if (func->getExistFuncCall()) {
        int64_t saveOffset = size - protectedRegNum * 8;

        if (saveOffset >= -512 && saveOffset <= 504 && (saveOffset % 8) == 0) {
            // 偏移量在有效范围内，直接使用ldp指令
            std::string validOff = "[sp, #" + std::to_string(saveOffset) + "]";
            emit("ldp", "x29", "x30", validOff);
        } else {
            // 偏移量超出范围，使用间接寻址
            // 先计算地址到临时寄存器
            load_imm(ARM64_TMP_REG_NO, saveOffset);
            emit("add",
                 PlatformArm64::regName[ARM64_TMP_REG_NO + 32],
                 "sp",
                 PlatformArm64::regName[ARM64_TMP_REG_NO + 32]);
            emit("ldp", "x29", "x30", "[" + PlatformArm64::regName[ARM64_TMP_REG_NO + 32] + "]");
        }
    }
    // 检查栈恢复大小是否超出add指令的立即数范围（0-4095）
    if (size <= 4095) {
        // 小栈帧，直接使用add指令
        std::string s = "#" + std::to_string(size);
        emit("add", "sp", "sp", s);
    } else {
        // 大栈帧，使用临时寄存器
        load_imm(ARM64_TMP_REG_NO, size);
        emit("add", "sp", "sp", PlatformArm64::regName[ARM64_TMP_REG_NO + 32]);
    }

    // 返回
    emit("ret");
}