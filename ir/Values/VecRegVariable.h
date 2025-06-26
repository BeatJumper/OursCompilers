///
/// @file VecRegVariable.h
/// @brief 向量寄存器变量类，用于后端
///
#pragma once

#include "Value.h"

/// @brief 向量寄存器Value
class VecRegVariable : public Value {

public:
    /// @brief 向量寄存器型Value
    /// \param val
    explicit VecRegVariable(Type * _type, std::string _name, int32_t _reg_no) : Value(_type)
    {
        this->name = _name;
        regId = _reg_no;
    }

    /// @brief 获取名字
    /// @return 变量名
    [[nodiscard]] std::string getIRName() const override
    {
        return name;
    }

private:
    /// @brief 寄存器编号，-1表示没有分配寄存器，大于等于0代表是寄存器型Value
    int32_t regId = -1;
};
