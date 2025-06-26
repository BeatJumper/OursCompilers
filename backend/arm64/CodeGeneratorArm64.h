///
/// @file CodeGeneratorArm64.h
/// @brief ARM64的后端处理头文件
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// </table>
///
#pragma once
#include "CodeGeneratorAsm.h"

class CodeGeneratorArm64 : public CodeGeneratorAsm {

public:
    /// @brief 构造函数
    /// @param tab 符号表
    CodeGeneratorArm64(Module * module);

    /// @brief 析构函数
    ~CodeGeneratorArm64() override;

protected:
    /// @brief 产生汇编头部分
    void genHeader() override;

    /// @brief 全局变量Section，主要包含初始化的和未初始化过的
    void genDataSection() override;

    /// @brief 针对函数进行汇编指令生成，放到.text代码段中
    /// @param func 要处理的函数
    void genCodeSection(Function * func) override;

    /// @brief 寄存器分配
    /// @param func 要处理的函数
    void registerAllocation(Function * func) override;

    /// @brief 基本块划分
    /// @param func 要处理的函数
    void GenBasicBlocks(Function * func);

    /// @brief 栈空间分配
    /// @param func 要处理的函数
    void stackAlloc(Function * func);

    /// @brief 寄存器分配前对函数内的指令进行调整，以便方便寄存器分配
    /// @param func 要处理的函数
    void adjustFuncCallInsts(Function * func);

    /// @brief 寄存器分配前对形参指令调整，便于栈内空间分配以及寄存器分配
    /// @param func 要处理的函数
    void adjustFormalParamInsts(Function * func);

    /// @brief 寄存器分配后对乘法和除法操作数里的常量添加赋值
    /// @param func 要处理的函数
    void adjustBinaryInsts(Function * func);

    /// @brief 寄存器分配前对常数进行扫描，对一些常数提前追加MOV指令
    /// @param func 要处理的函数
    void adjustMovInsts(Function * func);

    ///
    /// @brief 获取IR变量相关信息字符串
    /// @param str
    ///
    void getIRValueStr(Value * val, std::string & str);

private:
};
