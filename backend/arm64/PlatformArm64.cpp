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
    "w0",  "w1",  "w2",  "w3",  "w4",  "w5",  "w6",  "w7",  "w8",  "w9",  "w10", "w11", "w12", "w13", "w14", "w15",
    "w16", "w17", "w18", "w19", "w20", "w21", "w22", "w23", "w24", "w25", "w26", "w27", "w28", "x29", "x30", "sp",
    "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",  "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
    "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30"};

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
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[31], 31),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[32], 32),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[33], 33),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[34], 34),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[35], 35),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[36], 36),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[37], 37),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[38], 38),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[39], 39),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[40], 40),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[41], 41),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[42], 42),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[43], 43),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[44], 44),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[45], 45),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[46], 46),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[47], 47),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[48], 48),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[49], 49),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[50], 50),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[51], 51),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[52], 52),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[53], 53),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[54], 54),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[55], 55),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[56], 56),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[57], 57),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[58], 58),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[59], 59),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[60], 60),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[61], 61),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[62], 62),
};

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
