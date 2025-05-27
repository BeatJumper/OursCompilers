///
/// @file FloatType.cpp
/// @brief 浮点类型类，描述32位float类型
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

#include "FloatType.h"

///
/// @brief 唯一的Float类型实例
///
FloatType * FloatType::oneInstance = new FloatType();

///
/// @brief 获取类型，全局只有一份
/// @return FloatType*
///
FloatType * FloatType::getTypeFloat()
{
    return oneInstance;
}