///
/// @file ConstFloat.h
/// @brief 浮点型常量类，描述32位float常量
///
/// @author kangyk (2921006018@qq.com)
/// @version 1.0
/// @date 2025-05-25
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-05-25 <td>1.0     <td>kangyk  <td>新建
/// </table>
///

#pragma once

#include "Constant.h"
#include "FloatType.h"

///
/// @brief 浮点型常量类
///
class ConstFloat final : public Constant {

public:
    ///
    /// @brief 构造函数（指定浮点数值）
    /// @param val 浮点数值
    ///
    explicit ConstFloat(float val) : Constant(FloatType::getTypeFloat())
    {
        name = formatFloat(val);
        floatVal = val;
    }

    ///
    /// @brief 获取IR标识名称
    /// @return std::string
    ///
    [[nodiscard]] std::string getIRName() const override
    {
        return name;
    }

    ///
    /// @brief 获取浮点数值
    /// @return float
    ///
    [[nodiscard]] float getVal() const
    {
        return floatVal;
    }

private:
    ///
    /// @brief 格式化浮点数输出（LLVM IR格式）
    /// @param val 浮点数值
    /// @return std::string
    ///
    static std::string formatFloat(float val)
    {
        char buffer[32];
        // 使用科学计数法格式，这是LLVM IR要求的格式
        snprintf(buffer, sizeof(buffer), "%.6e", val);
        return buffer;
    }

    float floatVal;   ///< 存储的浮点数值
    std::string name; ///< IR显示的变量名
};