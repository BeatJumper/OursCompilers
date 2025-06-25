#pragma once

#include "Instruction.h"
#include "AllocaInstruction.h"

class StoreInstruction : public Instruction {
public:
    StoreInstruction(Function * _func, Value * _value, Value * _ptr, int _align = 4);
    void toString(std::string & str) override;

private:
    int align;
    Value * ptr;
};