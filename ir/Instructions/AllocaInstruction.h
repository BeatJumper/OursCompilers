#pragma once

#include "Instruction.h"

///
/// @brief AllocaInstruction class
///
class AllocaInstruction : public Instruction {
public:
    /// @brief Constructor
    /// @param _func The function to which this instruction belongs
    /// @param _result The result of the instruction
    /// @param _type The type of the instruction
    /// @param _align The alignment value (default: 4)
    AllocaInstruction(Function * _func, Value * _result, Type * _type, int _align = 4);

    /// @brief Convert instruction to string representation
    /// @param str Output string
    void toString(std::string & str) override;

    /// @brief 获取对齐值
    int getAligned();

private:
    int align; // 对齐值
};