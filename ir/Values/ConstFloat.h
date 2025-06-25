///
/// @file ConstFloat.h
/// @brief Float类型的常量
///

#pragma once

#include "Constant.h"
#include "IRConstant.h"
#include "FloatType.h"

///
/// @brief 整型常量类
///
class ConstFloat : public Constant {

public:
    ///
    /// @brief 指定值的常量
    /// \param val
    explicit ConstFloat(float val) : Constant(FloatType::getTypeFloat())
    {
        name = std::to_string(val);
        FloatVal = val;
    }

    /// @brief 获取名字
    /// @return 变量名
    [[nodiscard]] std::string getIRName() const override
    {
        return name;
    }

    ///
    /// @brief 获取值
    /// @return int32_t
    ///
    float getVal()
    {
        return FloatVal;
    }

private:
    ///
    /// @brief 整数值
    ///
    float FloatVal;
};