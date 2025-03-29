///
/// @file PlatformArm32.h
/// @brief  ARM64平台相关头文件
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

#include <string>
#include <cstdint>
#include "RegVariable.h"

// 在操作过程中临时借助的寄存器为ARM64_TMP_REG_NO
#define ARM64_TMP_REG_NO 10

// 栈寄存器SP和FP
#define ARM64_SP_REG_NO 31
#define ARM64_FP_REG_NO 29

// 函数跳转寄存器LX
#define ARM64_LX_REG_NO 30

/// @brief ARM64平台信息
class PlatformArm64 {

    /// @brief 循环左移两位
    /// @param num
    static void roundLeftShiftTwoBit(uint64_t & num);

    /// @brief 判断num是否是常数表达式，8位数字循环右移偶数位得到
    /// @param num
    /// @return
    static bool __constExpr(int64_t num);

public:
    /// @brief 同时处理正数和负数
    /// @param num
    /// @return
    static bool constExpr(int64_t num);

    /// @brief 判定是否是合法的偏移
    /// @param num
    /// @return
    static bool isDisp(int64_t num);

    /// @brief 判断是否是合法的寄存器名
    /// @param name 寄存器名字
    /// @return 是否是
    static bool isReg(std::string name);

    /// @brief 最大寄存器数目
    static const int maxRegNum = 32;

    /// @brief 可使用的通用寄存器的个数r0-r10
    static const int maxUsableRegNum = 29;

    /// @brief 寄存器的名字，r0-r15
    static const std::string regName[maxRegNum];

    /// @brief 对寄存器R0分配Value，记录位置
    static RegVariable * intRegVal[PlatformArm64::maxRegNum];
};
