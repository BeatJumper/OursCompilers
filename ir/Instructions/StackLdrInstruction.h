#pragma once

#include "Instruction.h"

class StackLdrInstruction : public Instruction {
public:
    StackLdrInstruction(Function * _func, Value * _value, Type * _type);
    void toString(std::string & str) override;

private:
    int align;

    /// @brief 所属的Value
    Value * val;
};