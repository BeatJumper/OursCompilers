///
/// @file FloatType.h
/// @brief 浮点类型类，描述32位float类型
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-22
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-22 <td>1.0     <td>zenglj  <td>新建
/// </table>
///

#pragma once

#include "Type.h"

class FloatType final : public Type {

public:
    ///
    /// @brief 获取类型，全局只有一份
    /// @return FloatType*
    ///
    static FloatType * getTypeFloat();

    ///
    /// @brief 获取类型的IR标识符
    /// @return std::string IR标识符
    ///
    [[nodiscard]] std::string toString() const override
    {
        return "float";
    }

    ///
    /// @brief 是否是浮点类型
    /// @return true
    /// @return false
    ///
    [[nodiscard]] bool isFloatType() const // override
    {
        return true;
    }

    ///
    /// @brief 获得类型所占内存空间大小
    /// @return int32_t
    ///
    [[nodiscard]] int32_t getSize() const override
    {
        return 4;
    }

private:
    ///
    /// @brief 构造函数
    ///
    explicit FloatType() : Type(Type::FloatTyID)
    {}

    ///
    /// @brief 唯一的Float类型实例
    ///
    static FloatType * oneInstance;
};
