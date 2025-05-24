///
/// @file XorInstruction.h
/// @brief XOR（异或）指令
///
#pragma once

#include "Instruction.h"

///
/// @brief XOR（异或）指令类
///
class XorInstruction : public Instruction {
public:
    /// @brief 构造函数
    /// @param _func 所属函数
    /// @param _srcVal1 源操作数1
    /// @param _srcVal2 源操作数2
    /// @param _type 结果类型
    XorInstruction(Function * _func, Value * _srcVal1, Value * _srcVal2, Type * _type);

    /// @brief 转换成字符串
    void toString(std::string & str) override;
};