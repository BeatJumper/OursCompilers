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

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    int32_t getLoadRegId() override
    {
        return this->loadRegNo;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    void setLoadRegId(int32_t regId) override
    {
        this->loadRegNo = regId;
    }

private:
    ///
    /// @brief 整数值
    ///
    float FloatVal;

    ///
    /// @brief 变量加载到寄存器中时对应的寄存器编号
    ///
    int32_t loadRegNo = -1;
};