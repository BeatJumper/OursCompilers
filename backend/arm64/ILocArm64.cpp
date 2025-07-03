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
    std::string reg_name = PlatformArm64::regName[rs_reg_no];

    // 对于地址计算，确保使用64位寄存器
    if (rs_reg_no >= 32 && reg_name[0] == 'w') {
        reg_name[0] = 'x';
    }

    // 检查立即数是否在16位范围内
    if (constant >= 0 && constant <= 65535) {
        // 小立即数，直接使用mov指令
        emit("mov", reg_name, "#" + std::to_string(constant));
    } else {
        // 大立即数，使用movz + movk指令组合
        uint32_t value = static_cast<uint32_t>(constant);
        uint16_t low16 = value & 0xFFFF;          // 低16位
        uint16_t high16 = (value >> 16) & 0xFFFF; // 高16位

        // 生成movz指令加载低16位
        char low16_hex[16];
        sprintf(low16_hex, "#0x%04X, lsl #0", low16);
        emit("movz", reg_name, low16_hex);

        // 如果高16位不为0，生成movk指令加载高16位
        if (high16 != 0) {
            char high16_hex[32];
            sprintf(high16_hex, "#0x%04X, lsl #16", high16);
            emit("movk", reg_name, high16_hex);
        }
    }
}

/*
    加载浮点数立即数
*/
void ILocArm64::load_float_imm(int rs_reg_no, float val)
{
    // 获取目标浮点寄存器名称
    std::string float_reg_name = PlatformArm64::regName[rs_reg_no];

    // 检查是否为特殊值
    if (val == 0.0f) {
        // 对于0.0，使用fmov从零寄存器加载
        emit("fmov", float_reg_name, "wzr");
        printf("Debug: load_float_imm - loaded 0.0 to %s using wzr\n", float_reg_name.c_str());
        return;
    }

    // 将float转换为uint32_t的位模式
    uint32_t bits;
    std::memcpy(&bits, &val, sizeof(float));

    // 使用临时整数寄存器来传递位模式
    std::string temp_int_reg = "w" + std::to_string(ARM64_TMP_REG_NO);

    // 先将位模式加载到临时通用寄存器
    load_imm(ARM64_TMP_REG_NO, bits);

    // 然后从通用寄存器移动到浮点寄存器
    emit("fmov", float_reg_name, temp_int_reg);

    printf("Debug: load_float_imm - loaded %f (bits=0x%08X) to %s via %s\n",
           val,
           bits,
           float_reg_name.c_str(),
           temp_int_reg.c_str());
}

/// @brief 加载符号值 ldr r0,=g ldr r0,=.L1
/// @param rs_reg_no 结果寄存器编号
/// @param name 符号名
void ILocArm64::load_symbol(int rs_reg_no, std::string name)
{
    // 检查寄存器编号有效性
    if (rs_reg_no < 0 || rs_reg_no >= PlatformArm64::maxRegNum) {
        printf("Error: load_symbol - invalid register number: %d\n", rs_reg_no);
        return;
    }

    std::string reg_name = PlatformArm64::regName[rs_reg_no];
    if (reg_name.empty()) {
        printf("Error: load_symbol - empty register name for regId: %d\n", rs_reg_no);
        return;
    }

    // adrp 指令加载符号所在页的基地址到指定寄存器
    emit("adrp", reg_name, name);

    // add 指令将符号在页内的偏移量加到基地址上
    // :lo12: 表示取符号地址的低 12 位作为偏移量
    emit("add", reg_name, reg_name, ":lo12:" + name);

    printf("Debug: load_symbol - loaded symbol %s to register %s\n", name.c_str(), reg_name.c_str());
}

/// @brief 基址寻址 ldr r0,[fp,#100]
/// @param rsReg 结果寄存器
/// @param base_reg_no 基址寄存器
/// @param offset 偏移
void ILocArm64::load_base(int rs_reg_no, int base_reg_no, int64_t offset)
{
    printf("Debug: load_base - 输入参数: rs_reg_no=%d, base_reg_no=%d, offset=%ld\n", rs_reg_no, base_reg_no, offset);

    std::string rsReg = PlatformArm64::regName[rs_reg_no];
    std::string base = PlatformArm64::regName[base_reg_no];

    // 确保基址寄存器是64位（ARM64 ldr指令要求）
    if (base[0] == 'w') {
        base[0] = 'x';
    }

    printf("Debug: load_base - 获取的寄存器名称: rsReg='%s', base='%s'\n", rsReg.c_str(), base.c_str());
    std::cout << "基址寻址中,结果寄存器" << rsReg << "\n";

    // 检查偏移量是否在ldr指令的有效范围内
    // ARM64 ldr指令的立即数偏移范围：0到4095（12位无符号）
    if (offset >= 0 && offset <= 4095) {
        // 有效的偏移常量
        if (offset) {
            // [fp,#-16] [fp]
            base += "," + toStr(offset);
        }
    } else {
        // 偏移量超出范围，使用临时寄存器计算地址
        // 使用临时寄存器避免破坏结果寄存器
        int temp_reg = ARM64_TMP_REG_NO;
        std::string temp_reg_name = PlatformArm64::regName[temp_reg + 32]; // 使用64位寄存器
        std::string base_reg_name = PlatformArm64::regName[base_reg_no];

        // 确保基址寄存器也是64位
        if (base_reg_name[0] == 'w') {
            base_reg_name[0] = 'x';
        }

        // 加载偏移量到临时寄存器（使用64位寄存器）
        load_imm(temp_reg + 32, offset);

        // 计算最终地址：temp_reg = base + offset
        emit("add", temp_reg_name, base_reg_name, temp_reg_name);

        // 使用计算出的地址进行加载
        base = "[" + temp_reg_name + "]";
        emit("ldr", rsReg, base);
        return; // 直接返回，避免后面的重复emit
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
    std::string src_reg_name;

    // 确保基址寄存器是64位（ARM64 str指令要求）
    if (base[0] == 'w') {
        base[0] = 'x';
    }

    // 处理特殊的零寄存器
    if (src_reg_no == 31) {
        src_reg_name = "wzr"; // 零寄存器
    } else {
        src_reg_name = PlatformArm64::regName[src_reg_no];
    }

    // 检查偏移量是否在str指令的有效范围内
    // ARM64 str指令的立即数偏移范围：0到4095（12位无符号）
    if (disp >= 0 && disp <= 4095) {
        // 有效的偏移常量

        // 若disp为0，则直接采用基址，否则采用基址+偏移
        // [fp,#-16] [fp]
        if (disp) {
            base += "," + toStr(disp);
        }
    } else {
        // 偏移量超出范围，使用临时寄存器计算地址
        std::string temp_reg_name = PlatformArm64::regName[tmp_reg_no + 32]; // 使用64位寄存器
        std::string base_reg_name = PlatformArm64::regName[base_reg_no];

        // 确保基址寄存器也是64位
        if (base_reg_name[0] == 'w') {
            base_reg_name[0] = 'x';
        }

        // 加载偏移量到临时寄存器（使用64位寄存器）
        load_imm(tmp_reg_no + 32, disp);

        // 计算最终地址：temp_reg = base + offset
        emit("add", temp_reg_name, base_reg_name, temp_reg_name);

        // 使用计算出的地址进行存储
        base = "[" + temp_reg_name + "]";
        emit("str", src_reg_name, base);
        return; // 直接返回，避免后面的重复emit
    }

    // 内存间接寻址
    base = "[" + base + "]";

    // str x8,[fp,#-8]
    // str x8,[fp,x9]
    emit("str", src_reg_name, base);
}

/// @brief 寄存器Mov操作
/// @param rs_reg_no 结果寄存器
/// @param src_reg_no 源寄存器
void ILocArm64::mov_reg(int rs_reg_no, int src_reg_no)
{
    std::string rs_reg_name = PlatformArm64::regName[rs_reg_no];
    std::string src_reg_name = PlatformArm64::regName[src_reg_no];

    // 检查是否为浮点寄存器移动
    if ((rs_reg_no >= 63 && rs_reg_no <= 126) || (src_reg_no >= 63 && src_reg_no <= 126)) {
        // 浮点寄存器使用fmov指令
        emit("fmov", rs_reg_name, src_reg_name);
        printf("Debug: mov_reg - fmov %s, %s (float registers)\n", rs_reg_name.c_str(), src_reg_name.c_str());
    } else {
        // 整数寄存器使用mov指令
        emit("mov", rs_reg_name, src_reg_name);
        printf("Debug: mov_reg - mov %s, %s (integer registers)\n", rs_reg_name.c_str(), src_reg_name.c_str());
    }
}

/// @brief 加载变量到寄存器，保证将变量放到reg中
/// @param rs_reg_no 结果寄存器
/// @param src_var 源操作数
void ILocArm64::load_var(int rs_reg_no, Value * src_var)
{
    printf("Debug: load_var - rs_reg_no=%d, src_var=%p\n", rs_reg_no, src_var);

    printf("Debug: load_var - src_var name=%s, IRName=%s\n", src_var->getName().c_str(), src_var->getIRName().c_str());

    if (Instanceof(constVal, ConstInt *, src_var)) {
        // 整型常量
        // mov w8,#100
        load_imm(rs_reg_no, constVal->getVal());
    } else if (Instanceof(constFloat, ConstFloat *, src_var)) {
        // 浮点数常量
        // 对于浮点数常量，需要将其加载到浮点寄存器
        load_float_imm(rs_reg_no, constFloat->getVal());
    } else if (src_var->getRegId() == -2) {
        // 处理溢出变量（regId == -2）
        printf("Debug: load_var - src_var is spilled variable\n");
        int32_t src_base_reg = -1;
        int64_t src_offset = -1;
        if (src_var->getMemoryAddr(&src_base_reg, &src_offset)) {
            printf("Debug: load_var - loading spilled variable from memory\n");
            load_base(rs_reg_no, src_base_reg, src_offset);
            return;
        } else {
            printf("Debug: load_var - ERROR: spilled variable has no memory address\n");
            minic_log(LOG_ERROR, "Spilled variable has no memory address");
        }
    } else if (src_var->getRegId() != -1) {

        int32_t src_base_reg = -1;
        int64_t src_offset = -1;
        if (src_var->getMemoryAddr(&src_base_reg, &src_offset)) {
            printf("Debug: load_var - src_var is in memory\n");
            load_base(rs_reg_no, src_base_reg, src_offset);
            return;
        }

        // 源操作数为寄存器变量
        // 检查这是否是一个指针类型（地址）还是普通值
        int32_t src_regId = src_var->getRegId();

        if (src_var->getType()->isPointerType()) {
            printf("Debug: load_var - src_var is pointer type, loading from address\n");
            // 对于指针类型，寄存器中存储的是地址，需要从地址加载数据

            // 确保使用64位寄存器进行地址访问
            std::string src_reg_name = PlatformArm64::regName[src_regId];
            std::string result_reg_name = PlatformArm64::regName[rs_reg_no];
            if (src_reg_name[0] == 'w') {
                src_reg_name[0] = 'x';
            }

            // ldr w8, [x2] - 从寄存器中的地址加载数据
            emit("ldr", result_reg_name, "[" + src_reg_name + "]");
        } else {
            printf("Debug: load_var - src_var is value type, direct register move\n");
            // 对于非指针类型，寄存器中存储的是值，直接移动寄存器
            std::string src_reg_name = PlatformArm64::regName[src_regId];
            std::string result_reg_name = PlatformArm64::regName[rs_reg_no];

            // 如果源和目标寄存器不同，则需要mov指令
            if (src_regId != rs_reg_no) {
                emit("mov", result_reg_name, src_reg_name);
            }
            // 如果相同，则不需要任何操作
        }
    } else if (Instanceof(globalVar, GlobalVariable *, src_var)) {
        // 全局变量

        // 读取全局变量的地址
        // 使用临时寄存器来加载符号地址
        int temp_reg = ARM64_TMP_REG_NO + 32; // 使用64位临时寄存器
        load_symbol(temp_reg, globalVar->getName());

        // ldr 指令从全局变量地址加载值
        std::string result_reg_name = PlatformArm64::regName[rs_reg_no];
        std::string temp_reg_name = PlatformArm64::regName[temp_reg];
        emit("ldr", result_reg_name, "[" + temp_reg_name + "]");

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
        // 使用临时寄存器来加载符号地址
        int temp_reg = ARM64_TMP_REG_NO + 32; // 使用64位临时寄存器
        load_symbol(temp_reg, globalVar->getName());

        // str 指令将值存储到全局变量地址
        std::string src_reg_name = PlatformArm64::regName[src_reg_no];
        std::string temp_reg_name = PlatformArm64::regName[temp_reg];
        emit("str", src_reg_name, "[" + temp_reg_name + "]");

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
    // 如果base_reg_no是-1，说明没有有效的基址寄存器，使用栈指针SP
    if (base_reg_no == -1) {
        base_reg_no = ARM64_SP_REG_NO; // 使用栈指针SP作为基址寄存器
    }

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
        std::string offset_str = toStr(-off);
        emit("sub", rs_reg_name, base_reg_name, offset_str);
    } else {
        // 偏移量超出范围，使用临时寄存器
        load_imm(rs_reg_no, off);
        emit("add", rs_reg_name, base_reg_name, rs_reg_name);
    }
}

/// @brief 生成函数序言(生成栈帧)
/// @param func 函数
/// @param tmp_reg_No
void ILocArm64::allocStack(Function * func, int tmp_reg_no)
{
    // 计算栈帧加上保护寄存器的栈空间总大小
    int64_t maxOffset = func->getMaxDep();

    int totalSize = maxOffset;
    int protectedRegNum = 0;

    // 保存寄存器空间
    if (func->getExistFuncCall()) {
        protectedRegNum = func->getProtectedReg().size();
        totalSize += protectedRegNum * 8;
    }

    // 对齐到16字节边界(ARM64要求)
    totalSize = (totalSize + 15) & ~15;

    printf("生成函数序言,总栈空间大小:%d\n", totalSize);

    func->setStackFrameSize(totalSize);

    // 检查栈空间大小是否超出立即数范围
    if (totalSize <= 4095) {
        // 栈空间在立即数范围内，直接使用sub指令
        std::string s = "#" + std::to_string(totalSize);
        emit("sub", "sp", "sp", s);
    } else {
        // 栈空间超出立即数范围，使用临时寄存器
        load_imm(tmp_reg_no, totalSize);
        std::string tmp_reg_name = PlatformArm64::regName[tmp_reg_no];
        // 确保使用64位寄存器
        if (tmp_reg_name[0] == 'w') {
            tmp_reg_name[0] = 'x';
        }
        emit("sub", "sp", "sp", tmp_reg_name);
    }

    if (func->getExistFuncCall()) {
        auto & protectedRegNo = func->getProtectedReg();
        for (int i = 0; i < protectedRegNo.size(); i++) {
            int64_t offset = totalSize - (protectedRegNum - i) * 8;
            // 检查偏移量是否在str指令的有效范围内
            if ((offset >= -256 && offset <= 255) || (offset >= 0 && offset <= 16380 && (offset % 8) == 0)) {
                std::string off = "[sp, #" + std::to_string(offset) + "]";
                emit("str", PlatformArm64::intRegVal[protectedRegNo[i]]->getName(), off);
            } else {
                // 偏移量超出范围，使用临时寄存器
                load_imm(tmp_reg_no, offset);
                std::string tmp_reg_name = PlatformArm64::regName[tmp_reg_no];
                // 确保使用64位寄存器作为偏移
                if (tmp_reg_name[0] == 'w') {
                    tmp_reg_name[0] = 'x';
                }
                std::string off = "[sp, " + tmp_reg_name + "]";
                emit("str", PlatformArm64::intRegVal[protectedRegNo[i]]->getName(), off);
            }
        }
        // 设置新帧指针
        int64_t fpOffset = totalSize - protectedRegNum * 8;
        if (fpOffset <= 4095) {
            emit("add", "x29", "sp", "#" + std::to_string(fpOffset));
        } else {
            load_imm(tmp_reg_no, fpOffset);
            std::string tmp_reg_name = PlatformArm64::regName[tmp_reg_no];
            // 确保使用64位寄存器
            if (tmp_reg_name[0] == 'w') {
                tmp_reg_name[0] = 'x';
            }
            emit("add", "x29", "sp", tmp_reg_name);
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

/// @brief 生成neg指令（取负）
/// @param dest_reg 目标寄存器编号
/// @param src_reg 源寄存器编号
void ILocArm64::neg(int dest_reg, int src_reg)
{
    emit("neg", PlatformArm64::regName[dest_reg], PlatformArm64::regName[src_reg]);
}

/// @brief 生成函数结尾(恢复栈帧)
void ILocArm64::emitFunctionEpilogue(Function * func)
{
    int size = func->getStackFrameSize();
    int protectedRegNum = 0;
    if (func->getExistFuncCall()) {
        protectedRegNum = func->getProtectedReg().size();
    }

    // 恢复所有保护寄存器
    if (func->getExistFuncCall()) {
        auto & protectedRegNo = func->getProtectedReg();
        for (int i = 0; i < protectedRegNo.size(); i++) {
            int64_t offset = size - (protectedRegNum - i) * 8;
            // 检查偏移量是否在ldr指令的有效范围内
            if ((offset >= -256 && offset <= 255) || (offset >= 0 && offset <= 16380 && (offset % 8) == 0)) {
                std::string off = "[sp, #" + std::to_string(offset) + "]";
                emit("ldr", PlatformArm64::intRegVal[protectedRegNo[i]]->getName(), off);
            } else {
                // 偏移量超出范围，使用临时寄存器
                // 使用x9作为临时寄存器（ARM64_TMP_REG_NO对应的寄存器）
                load_imm(ARM64_TMP_REG_NO, offset);
                std::string tmp_reg_name = PlatformArm64::regName[ARM64_TMP_REG_NO];
                // 确保使用64位寄存器作为偏移
                if (tmp_reg_name[0] == 'w') {
                    tmp_reg_name[0] = 'x';
                }
                std::string off = "[sp, " + tmp_reg_name + "]";
                emit("ldr", PlatformArm64::intRegVal[protectedRegNo[i]]->getName(), off);
            }
        }
    }

    // 检查栈空间大小是否超出立即数范围
    if (size <= 4095) {
        // 栈空间在立即数范围内，直接使用add指令
        std::string s = "#" + std::to_string(size);
        emit("add", "sp", "sp", s);
    } else {
        // 栈空间超出立即数范围，使用临时寄存器
        load_imm(ARM64_TMP_REG_NO, size);
        std::string tmp_reg_name = PlatformArm64::regName[ARM64_TMP_REG_NO];
        // 确保使用64位寄存器
        if (tmp_reg_name[0] == 'w') {
            tmp_reg_name[0] = 'x';
        }
        emit("add", "sp", "sp", tmp_reg_name);
    }

    // 返回
    emit("ret");
}