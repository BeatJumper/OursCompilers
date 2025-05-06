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

const std::string PlatformArm64::regName[PlatformArm64::maxRegNum] = {
    "x0",  // 用于传参或返回值等
    "x1",  // 用于传参或返回值等
    "x2",  // 用于传参等
    "x3",  // 用于传参等
    "x4",  // 用于传参等
    "x5",  // 用于传参等
    "x6",  // 用于传参等
    "x7",  // 用于传参等
    "x8",  // 用于特定操作，如系统调用返回地址等
    "x9",  // 通用寄存器
    "x10", // 通用寄存器
    "x11", // 通用寄存器
    "x12", // 通用寄存器
    "x13", // 通用寄存器
    "x14", // 通用寄存器
    "x15", // 通用寄存器
    "x16", // 临时寄存器
    "x17", // 临时寄存器
    "x18", // 平台保留寄存器
    "x19", // 通用寄存器
    "x20", // 通用寄存器
    "x21", // 通用寄存器
    "x22", // 通用寄存器
    "x23", // 通用寄存器
    "x24", // 通用寄存器
    "x25", // 通用寄存器
    "x26", // 通用寄存器
    "x27", // 通用寄存器
    "x28", // 通用寄存器
    "fp",  // x29,帧指针
    "lr",  // x30，链接寄存器
    "sp",  // 堆栈指针寄存器
};

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
    new RegVariable(IntegerType::getTypeInt(), PlatformArm64::regName[31], 31)
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
    for (const auto& reg : regName) {
        if (reg == name) {
            return true;
        }
    }
    return false;
}
