///
/// @file ConstInt.h
/// @brief int类型的常量
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

#include "Constant.h"
#include "IRConstant.h"
#include "IntegerType.h"

///
/// @brief 整型常量类
///
class ConstInt : public Constant {

public:
    ///
    /// @brief 指定值的常量（32位）
    /// \param val
    explicit ConstInt(int32_t val) : Constant(IntegerType::getTypeInt())
    {
        name = std::to_string(val);
        intVal = val;
        is64Bit = false;
    }

    ///
    /// @brief 指定类型和值的常量（32位）
    /// \param type 类型
    /// \param val 值
    ConstInt(Type * type, int32_t val) : Constant(type)
    {
        name = std::to_string(val);
        intVal = val;
        is64Bit = false;
    }

    ///
    /// @brief 指定值的常量（64位）
    /// \param val 64位值
    explicit ConstInt(int64_t val) : Constant(IntegerType::getTypeLong())
    {
        name = std::to_string(val);
        longVal = val;
        is64Bit = true;
    }

    ///
    /// @brief 指定类型和值的常量（64位）
    /// \param type 类型（应该是64位整数类型）
    /// \param val 64位值
    ConstInt(Type * type, int64_t val) : Constant(type)
    {
        name = std::to_string(val);
        longVal = val;
        is64Bit = true;
    }

    /// @brief 获取名字
    /// @return 变量名
    [[nodiscard]] std::string getIRName() const override
    {
        return name;
    }

    ///
    /// @brief 获取32位值
    /// @return int32_t
    ///
    int32_t getVal()
    {
        if (is64Bit) {
            // 如果是64位值，返回截断后的32位值（或抛出异常）
            return static_cast<int32_t>(longVal);
        }
        return intVal;
    }

    ///
    /// @brief 获取64位值
    /// @return int64_t
    ///
    int64_t getLongVal()
    {
        if (is64Bit) {
            return longVal;
        }
        return static_cast<int64_t>(intVal);
    }

    ///
    /// @brief 判断是否是64位常量
    /// @return bool
    ///
    bool is64BitConstant() const
    {
        return is64Bit;
    }

private:
    ///
    /// @brief 32位整数值
    ///
    int32_t intVal = 0;

    ///
    /// @brief 64位整数值
    ///
    int64_t longVal = 0;

    ///
    /// @brief 是否是64位常量
    ///
    bool is64Bit = false;
};