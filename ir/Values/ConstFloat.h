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
#include <cinttypes>

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
        // 使用十六进制格式，这是LLVM IR要求的IEEE 754表示
        // 对于32位float，LLVM期望16位十六进制数字（64位表示）
        uint32_t bits = *reinterpret_cast<uint32_t *>(&val);
        // 将32位float值放在64位表示的高32位
        uint64_t extended_bits = static_cast<uint64_t>(bits) << 32;
        snprintf(buffer, sizeof(buffer), "0x%016" PRIx64, extended_bits);
        return buffer;
    }

    float floatVal;   ///< 存储的浮点数值
    std::string name; ///< IR显示的变量名
};