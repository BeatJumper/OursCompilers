#pragma once

#include "Instruction.h"

class StackStrInstruction : public Instruction {
public:
    StackStrInstruction(Function * _func, Value * _value);
    void toString(std::string & str) override;

private:
    int align;
};