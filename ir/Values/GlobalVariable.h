///
/// @file GlobalVariable.h
/// @brief 全局变量描述类
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#pragma once

#include "GlobalValue.h"
#include "IRConstant.h"

///
/// @brief 全局变量，寻址时通过符号名或变量名来寻址
///
class GlobalVariable : public GlobalValue {

public:
    ///
    /// @brief 构建全局变量，默认对齐为4字节
    /// @param _type 类型
    /// @param _name 名字
    ///
    explicit GlobalVariable(Type * _type, std::string _name) : GlobalValue(_type, _name)
    {
        // 设置对齐大小
        setAlignment(4);
    }

    ///
    /// @brief  检查是否是函数
    /// @return true 是函数
    /// @return false 不是函数
    ///
    [[nodiscard]] bool isGlobalVarible() const override
    {
        return true;
    }

    ///
    /// @brief 是否属于BSS段的变量，即未初始化过的变量，或者初值都为0的变量
    /// @return true
    /// @return false
    ///
    [[nodiscard]] bool isInBSSSection() const
    {
        return this->inBSSSection;
    }

    ///
    /// @brief 取得变量所在的作用域层级
    /// @return int32_t 层级
    ///
    int32_t getScopeLevel() override
    {
        return 0;
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

    ///
    /// @brief Declare指令IR显示
    /// @param str
    ///
    void toDeclareString(std::string & str)
    {
        // str = "declare " + getType()->toString() + " " + getIRName();

        // TODO:应该改成下面这种格式
        // @a = dso_local global i32 33, align 4
        // 33是初始值

        str += "@" + getName() + " = dso_local ";

        // 根据是否为常量选择关键字
        if (isConstant) {
            str += "constant ";
        } else {
            str += "global ";
        }

        str += getType()->toString();

        // 处理初值
        if (initValue) {
            // 检查初值类型并生成相应的字符串表示
            if (dynamic_cast<ConstInt *>(initValue)) {
                ConstInt * constInt = static_cast<ConstInt *>(initValue);
                str += " " + std::to_string(constInt->getVal());
            } else {
                // 如果不是ConstInt，使用默认值0
                str += " 0";
            }
        } else {
            // 没有初值，使用默认值0
            str += " 0";
        }

        str += ", align " + std::to_string(getAlignment());
    }

    ///
    /// @brief 设置初值
    /// @param val 初值
    ///
    void setInitValue(Value * val)
    {
        initValue = val;
        if (val && !dynamic_cast<ConstInt *>(val)) {
            inBSSSection = false; // 如果有非零初值，不在BSS段
        }
    }

    ///
    /// @brief 获取初值
    /// @return 初值
    ///
    Value * getInitValue() const
    {
        return initValue;
    }

    ///
    /// @brief 设置是否是常量
    /// @param isConst 是否是常量
    ///
    void setConstant(bool isConst)
    {
        isConstant = isConst;
    }

    ///
    /// @brief 检查是否是常量
    /// @return true 是常量
    ///
    [[nodiscard]] bool getConstant() const
    {
        return isConstant;
    }

private:
    ///
    /// @brief 变量加载到寄存器中时对应的寄存器编号
    ///
    int32_t loadRegNo = -1;

    ///
    /// @brief 默认全局变量在BSS段，没有初始化，或者即使初始化过，但都值都为0
    ///
    bool inBSSSection = true;

    ///
    /// @brief 全局变量的初值
    ///
    Value * initValue = nullptr;

    ///
    /// @brief 常量标记
    ///
    bool isConstant = false;
};
