///
/// @file PlatformArm32.cpp
/// @brief  ARM32平台相关实现
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
#include "PlatformArm64.h"

#include "IntegerType.h"
#include "FloatType.h"

const std::string PlatformArm64::regName[PlatformArm64::maxRegNum] = {
    "w0",  // 用于传参或返回值等
    "w1",  // 用于传参或返回值等
    "w2",  // 用于传参等
    "w3",  // 用于传参等
    "w4",  // 用于传参等
    "w5",  // 用于传参等
    "w6",  // 用于传参等
    "w7",  // 用于传参等
    "w8",  // 用于特定操作，如系统调用返回地址等
    "w9",  // 通用寄存器
    "w10", // 通用寄存器
    "w11", // 通用寄存器
    "w12", // 通用寄存器
    "w13", // 通用寄存器
    "w14", // 通用寄存器
    "w15", // 通用寄存器
    "w16", // 临时寄存器
    "w17", // 临时寄存器
    "w18", // 平台保留寄存器
    "w19", // 通用寄存器
    "w20", // 通用寄存器
    "w21", // 通用寄存器
    "w22", // 通用寄存器
    "w23", // 通用寄存器
    "w24", // 通用寄存器
    "w25", // 通用寄存器
    "w26", // 通用寄存器
    "w27", // 通用寄存器
    "w28", // 通用寄存器
    "x29", // x29,帧指针
    "x30", // x30，链接寄存器
    "sp",  // 堆栈指针寄存器
};

// 向量寄存器
const std::string PlatformArm64::vecRegName[PlatformArm64::maxVecRegNum] = {
    "v0",  "v1",  "v2",  "v3",  "v4",  "v5",  "v6",  "v7",  "v8",  "v9",  "v10", "v11", "v12", "v13", "v14", "v15",
    "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"};

// 状态寄存器
const std::string PlatformArm64::statusRegName = "CPSR";

StatusRegVariable * PlatformArm64::statusRegVal =
    new StatusRegVariable(IntegerType::getTypeInt(), PlatformArm64::statusRegName, 0);

RegVariable * PlatformArm64::intRegVal[PlatformArm64::maxRegNum] = {
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[0], 0),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[1], 1),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[2], 2),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[3], 3),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[4], 4),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[5], 5),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[6], 6),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[7], 7),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[8], 8),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[9], 9),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[10], 10),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[11], 11),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[12], 12),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[13], 13),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[14], 14),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[15], 15),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[16], 16),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[17], 17),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[18], 18),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[19], 19),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[20], 20),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[21], 21),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[22], 22),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[23], 23),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[24], 24),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[25], 25),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[26], 26),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[27], 27),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[28], 28),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[29], 29),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[30], 30),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[31], 31)};

VecRegVariable * PlatformArm64::VecRegVal[PlatformArm64::maxVecRegNum] = {
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[0], 0),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[1], 1),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[2], 2),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[3], 3),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[4], 4),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[5], 5),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[6], 6),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[7], 7),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[8], 8),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[9], 9),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[10], 10),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[11], 11),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[12], 12),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[13], 13),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[14], 14),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[15], 15),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[16], 16),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[17], 17),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[18], 18),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[19], 19),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[20], 20),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[21], 21),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[22], 22),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[23], 23),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[24], 24),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[25], 25),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[26], 26),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[27], 27),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[28], 28),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[29], 29),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[30], 30),
    new VecRegVariable(FloatType::getTypeFloat(), PlatformArm64::vecRegName[31], 31)};

/// @brief 循环左移两位
/// @param num
void PlatformArm64::roundLeftShiftTwoBit(uint64_t & num)
{
    // 取左移即将溢出的两位
    const uint64_t overFlow = num & 0xc000000000000000;

    // 将溢出部分追加到尾部
    num = (num << 2) | (overFlow >> 62);
}

/// @brief 判断num是否是常数表达式，8位数字循环右移偶数位得到
/// @param num
/// @return
bool PlatformArm64::__constExpr(int64_t num)
{
    uint64_t new_num = (uint64_t) num;

    for (int i = 0; i < 32; i++) {

        if (new_num <= 0xff) {
            // 有效表达式
            return true;
        }

        // 循环左移2位
        roundLeftShiftTwoBit(new_num);
    }

    return false;
}

/// @brief 同时处理正数和负数
/// @param num
/// @return
bool PlatformArm64::constExpr(int64_t num)
{
    return __constExpr(num) || __constExpr(-num);
}

/// @brief 判定是否是合法的偏移
/// @param num
/// @return
bool PlatformArm64::isDisp(int64_t num)
{
    return num < 4096 && num > -4096;
}

/// @brief 判断是否是合法的寄存器名
/// @param s 寄存器名字
/// @return 是否是
bool PlatformArm64::isReg(std::string name)
{
    for (const auto & reg: regName) {
        if (reg == name) {
            return true;
        }
    }
    return false;
}
