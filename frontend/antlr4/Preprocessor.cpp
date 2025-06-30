///
/// @file Preprocessor.cpp
/// @brief 简单的预处理器实现文件，用于处理#define宏定义
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

#include "Preprocessor.h"
#include <sstream>
#include <regex>
#include <cctype>

/// @brief 处理源代码中的#define宏定义
/// @param sourceCode 输入的源代码
/// @return 处理后的源代码（宏已被替换）
std::string Preprocessor::process(const std::string & sourceCode)
{
    std::istringstream iss(sourceCode);
    std::ostringstream oss;
    std::string line;
    int lineNumber = 1;

    // 第一遍：解析所有的#define指令，同时处理starttime()和stoptime()
    while (std::getline(iss, line)) {
        if (line.find("#define") == 0) {
            // 解析#define指令
            if (parseDefine(line)) {
                // 成功解析，跳过这一行（不输出到结果中）
                lineNumber++;
                continue;
            }
        }

        // 处理starttime()和stoptime()宏调用
        line = processTimingMacros(line, lineNumber);

        // 处理数组定义中的单个{0}初始化
        line = processArrayZeroInitialization(line);

        // 保留处理后的行
        oss << line << "\n";
        lineNumber++;
    }

    // 第二遍：替换所有的宏
    std::string processedCode = oss.str();
    return replaceMacros(processedCode);
}

/// @brief 解析#define指令
/// @param line 包含#define的行
/// @return 是否成功解析
bool Preprocessor::parseDefine(const std::string & line)
{
    // 使用正则表达式解析 #define MACRO_NAME MACRO_VALUE
    std::regex defineRegex(R"(^\s*#define\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+)\s*$)");
    std::smatch match;

    if (std::regex_match(line, match, defineRegex)) {
        std::string macroName = match[1].str();
        std::string macroValue = match[2].str();

        // 去除宏值末尾的空白字符
        while (!macroValue.empty() && std::isspace(macroValue.back())) {
            macroValue.pop_back();
        }

        // 存储宏定义
        macroDefinitions[macroName] = macroValue;
        return true;
    }

    return false;
}

/// @brief 在文本中替换宏
/// @param text 要处理的文本
/// @return 替换后的文本
std::string Preprocessor::replaceMacros(const std::string & text)
{
    std::string result = text;

    // 对每个宏定义进行替换
    for (const auto & macro: macroDefinitions) {
        const std::string & macroName = macro.first;
        const std::string & macroValue = macro.second;

        size_t pos = 0;
        while ((pos = result.find(macroName, pos)) != std::string::npos) {
            // 检查前后字符，确保是完整的标识符
            bool validStart = (pos == 0) || !isIdentifierChar(result[pos - 1]);
            bool validEnd =
                (pos + macroName.length() >= result.length()) || !isIdentifierChar(result[pos + macroName.length()]);

            if (validStart && validEnd) {
                // 进行替换
                result.replace(pos, macroName.length(), macroValue);
                pos += macroValue.length();
            } else {
                pos += macroName.length();
            }
        }
    }

    return result;
}

/// @brief 处理starttime()和stoptime()宏调用
/// @param line 要处理的行
/// @param lineNumber 当前行号
/// @return 处理后的行
std::string Preprocessor::processTimingMacros(const std::string & line, int lineNumber)
{
    std::string result = line;

    // 处理starttime()调用
    std::regex starttimeRegex(R"(\bstarttime\s*\(\s*\))");
    std::string starttimeReplacement = "_sysy_starttime(" + std::to_string(lineNumber) + ")";
    result = std::regex_replace(result, starttimeRegex, starttimeReplacement);

    // 处理stoptime()调用
    std::regex stoptimeRegex(R"(\bstoptime\s*\(\s*\))");
    std::string stoptimeReplacement = "_sysy_stoptime(" + std::to_string(lineNumber) + ")";
    result = std::regex_replace(result, stoptimeRegex, stoptimeReplacement);

    return result;
}

/// @brief 处理数组定义中的单个{0}初始化
/// @param line 要处理的行
/// @return 处理后的行
std::string Preprocessor::processArrayZeroInitialization(const std::string & line)
{
    // 使用正则表达式匹配数组定义中只有单个{0}的初始化
    // 匹配模式：类型 变量名[维度]... = {0};
    // 注意：只处理等号右边恰好是{0}的情况，不处理复杂的嵌套初始化

    std::regex arrayZeroInitRegex(
        R"(^(\s*(?:int|float|const\s+int|const\s+float)\s+[a-zA-Z_][a-zA-Z0-9_]*(?:\[[^\]]*\])+)\s*=\s*\{0\}\s*;(.*)$)");

    std::smatch match;
    if (std::regex_match(line, match, arrayZeroInitRegex)) {
        // 找到匹配的数组定义，移除 = {0} 部分
        std::string arrayDecl = match[1].str();  // 数组声明部分
        std::string restOfLine = match[2].str(); // 行的其余部分（如果有的话）

        // 重新组合，去掉 = {0}
        return arrayDecl + ";" + restOfLine;
    }

    // 如果没有匹配，返回原行
    return line;
}

/// @brief 检查字符是否为标识符字符
/// @param c 要检查的字符
/// @return 是否为标识符字符
bool Preprocessor::isIdentifierChar(char c)
{
    return std::isalnum(c) || c == '_';
}
