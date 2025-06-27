///
/// @file Preprocessor.h
/// @brief 简单的预处理器头文件，用于处理#define宏定义
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-12-27
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-12-27 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#pragma once

#include <string>
#include <unordered_map>

/// @brief 简单的预处理器类，用于处理#define宏定义
class Preprocessor {

public:
    /// @brief 构造函数
    Preprocessor() = default;

    /// @brief 析构函数
    ~Preprocessor() = default;

    /// @brief 处理源代码中的#define宏定义
    /// @param sourceCode 输入的源代码
    /// @return 处理后的源代码（宏已被替换）
    std::string process(const std::string& sourceCode);

private:
    /// @brief 宏定义表，存储宏名称和对应的值
    std::unordered_map<std::string, std::string> macroDefinitions;

    /// @brief 解析#define指令
    /// @param line 包含#define的行
    /// @return 是否成功解析
    bool parseDefine(const std::string& line);

    /// @brief 在文本中替换宏
    /// @param text 要处理的文本
    /// @return 替换后的文本
    std::string replaceMacros(const std::string& text);

    /// @brief 检查字符是否为标识符字符
    /// @param c 要检查的字符
    /// @return 是否为标识符字符
    bool isIdentifierChar(char c);
};
