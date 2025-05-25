///
/// @file FloatType.cpp
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

#include "FloatType.h"

///
/// @brief 唯一的Float类型实例
///
FloatType * FloatType::oneInstance = new FloatType();

FloatType * FloatType::getTypeFloat()
{
    return oneInstance;
}