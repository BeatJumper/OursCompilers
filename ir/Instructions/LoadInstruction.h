#pragma once

#include "Instruction.h"

class LoadInstruction : public Instruction {
public:
    LoadInstruction(Function * _func, Value * _result, Value * _ptr, int _align = 4);
    void toString(std::string & str) override;

private:
    int align;
};