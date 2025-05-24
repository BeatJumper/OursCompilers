///
/// @file ZextInstruction.h
/// @brief ZEXT（零扩展）指令
///
#pragma once

#include "Instruction.h"

///
/// @brief ZEXT（零扩展）指令类
///
class ZextInstruction : public Instruction {
public:
    /// @brief 构造函数
    /// @param _func 所属函数
    /// @param _srcVal 源操作数
    /// @param _targetType 目标类型
    ZextInstruction(Function * _func, Value * _srcVal, Type * _targetType);

    /// @brief 转换成字符串
    void toString(std::string & str) override;
};