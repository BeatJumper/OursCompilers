///
/// @file IRGenerator.h
/// @brief AST遍历产生线性IR的头文件
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///
#pragma once

#include <unordered_map>
#include <stack>
#include <map>
#include <LabelInstruction.h>

#include "AST.h"
#include "Module.h"
#include "IRCode.h"
#include "ArrayType.h"

/// @brief AST遍历产生线性IR类
class IRGenerator {

public:
    /// @brief 构造函数
    /// @param root
    /// @param _module
    IRGenerator(ast_node * root, Module * _module);

    /// @brief 析构函数
    ~IRGenerator() = default;

    /// @brief 运行产生IR
    bool run();

protected:
    /// @brief 编译单元AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_compile_unit(ast_node * node);

    bool parseArrayInitializer(ast_node * initNode, const Type * arrayType, Value *& initValue);

    /// @brief 函数定义AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_function_define(ast_node * node);

    /// @brief 形式参数AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_function_formal_params(ast_node * node);

    /// @brief 函数调用AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_function_call(ast_node * node);

    /// @brief 语句块（含函数体）AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_block(ast_node * node);

    bool ir_add_or_fadd(ast_node * node);
    bool ir_add_processed(ast_node * node, ast_node * left, ast_node * right);
    bool ir_fadd_processed(ast_node * node, ast_node * left, ast_node * right);

    bool ir_sub_or_fsub(ast_node * node);
    bool ir_sub_processed(ast_node * node, ast_node * left, ast_node * right);
    bool ir_fsub_processed(ast_node * node, ast_node * left, ast_node * right);

    bool ir_mul_or_fmul(ast_node * node);
    bool ir_mul_processed(ast_node * node, ast_node * left, ast_node * right);
    bool ir_fmul_processed(ast_node * node, ast_node * left, ast_node * right);

    bool ir_div_or_fdiv(ast_node * node);
    bool ir_div_processed(ast_node * node, ast_node * left, ast_node * right);
    bool ir_fdiv_processed(ast_node * node, ast_node * left, ast_node * right);

    /// @brief 整数加法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_add(ast_node * node);

    /// @brief 整数减法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_sub(ast_node * node);

    /// @brief 整数乘法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_mul(ast_node * node);

    /// @brief 整数除法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_div(ast_node * node);

    /// @brief 赋值AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_assign(ast_node * node);

    Value * convertToFloat(Value * val, Function * func, InterCode & blockInsts);
    Value * convertToInt(Value * val, Function * func, InterCode & blockInsts);
    /// @brief return节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_return(ast_node * node);

    /// @brief 类型叶子节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_leaf_node_type(ast_node * node);

    /// @brief 标识符叶子节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_leaf_node_var_id(ast_node * node);

    /// @brief 无符号整数字面量叶子节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_leaf_node_uint(ast_node * node);

    /// @brief float数字面量叶子节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_leaf_node_float(ast_node * node);

    /// @brief 变量声明语句节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_declare_statment(ast_node * node);

    /// @brief 变量定声明节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_variable_declare(ast_node * node);

    /// @brief 未知节点类型的节点处理
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_default(ast_node * node);

    /// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
    /// @param node AST节点
    /// @return 成功返回node节点，否则返回nullptr
    ast_node * ir_visit_ast_node(ast_node * node);

    /// @brief 关系表达式AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_rel_exp(ast_node * node);

    /// @brief while语句AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_while(ast_node * node);

    /// @brief if-else语句AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_if_else(ast_node * node);

    /// @brief break语句AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_break(ast_node * node);

    /// @brief continue语句AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_continue(ast_node * node);

    /// @brief AST的节点操作函数
    typedef bool (IRGenerator::*ast2ir_handler_t)(ast_node *);

    /// @brief AST节点运算符与动作函数关联的映射表
    std::unordered_map<ast_operator_type, ast2ir_handler_t> ast2ir_handlers;

    /// @brief 处理算术运算中的类型转换（i1到i32的零扩展）
    /// @param node AST节点
    /// @param leftValue 左操作数值（可能被修改）
    /// @param rightValue 右操作数值（可能被修改）
    /// @return 是否成功处理类型转换
    bool handleArithmeticTypeConversion(ast_node * node, Value *& leftValue, Value *& rightValue);

private:
    /// @brief 抽象语法树的根
    ast_node * root;

    /// @brief 符号表:模块
    Module * module;

    /// @brief 循环标签栈，为了break/continue语句服务
    struct LoopLabels {
        LabelInstruction * condLabel;
        LabelInstruction * exitLabel;
    };
    std::stack<LoopLabels> loopLabelStack;

    /// @brief 判断一个值是否需要加载操作
    /// @param val 要检查的值
    /// @return true: 需要加载，false: 不需要加载
    bool needsLoad(Value * val);

    /// @brief 全局变量声明处理
    /// @param node AST节点
    /// @param typeNode 类型节点
    /// @param varOrAssignNode 变量或赋值节点
    /// @return 翻译是否成功
    bool ir_global_variable_declare(ast_node * node, ast_node * typeNode, ast_node * varOrAssignNode);

    /// @brief 整数取模AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_mod(ast_node * node);

    /// @brief 正号一元运算符AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_positive(ast_node * node);

    /// @brief 负号一元运算符AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_negative(ast_node * node);

    /// @brief 逻辑非运算符AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_not(ast_node * node);

    /// @brief 逻辑与运算符AST节点翻译成线性中间IR（支持短路求值）
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_and(ast_node * node);

    /// @brief 逻辑或运算符AST节点翻译成线性中间IR（支持短路求值）
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_or(ast_node * node);

private:
    /// @brief 将值转换为i1类型（如果需要）
    Value * convertToI1(Value * val, Function * func, InterCode & blockInsts);

    /// @brief 将i1类型的值扩展为i32类型（如果需要）
    Value * convertToI32(Value * val, Function * func, InterCode & blockInsts);

    /// @brief 逻辑与运算符AST节点翻译成线性中间IR（条件跳转版本）
    /// @param node AST节点
    /// @param trueLabel 真出口标签
    /// @param falseLabel 假出口标签
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_and_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel);

    /// @brief 逻辑或运算符AST节点翻译成线性中间IR（条件跳转版本）
    /// @param node AST节点
    /// @param trueLabel 真出口标签
    /// @param falseLabel 假出口标签
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_or_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel);

    /// @brief 逻辑非运算符AST节点翻译成线性中间IR（条件跳转版本）
    /// @param node AST节点
    /// @param trueLabel 真出口标签
    /// @param falseLabel 假出口标签
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_not_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel);

    /// @brief 关系表达式AST节点翻译成线性中间IR（条件跳转版本）
    /// @param node AST节点
    /// @param trueLabel 真出口标签
    /// @param falseLabel 假出口标签
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_rel_exp_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel);

    /// @brief 处理条件表达式，根据节点类型选择合适的处理方法
    /// @param node AST节点
    /// @param trueLabel 真出口标签
    /// @param falseLabel 假出口标签
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_condition_expr(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel);

    /// @brief 检查指令序列是否包含终结指令（如break、continue、return）
    /// @param blockInsts 指令序列
    /// @return true：包含终结指令，false：不包含
    bool hasTerminatorInstruction(const InterCode & blockInsts);

    /// @brief 检查指令序列是否包含return指令
    /// @param blockInsts 指令序列
    /// @return true：包含return指令，false：不包含
    bool hasReturnInstruction(const InterCode & blockInsts);

    /// @brief 重新组织初始化值以匹配目标数组类型
    /// @param flatValues 扁平化的初始化值
    /// @param targetType 目标数组类型
    /// @param reorganizedValues 输出的重新组织后的值
    /// @return 是否成功
    bool reorganizeInitValuesForTargetType(const std::vector<Value *> & flatValues,
                                           ArrayType * targetType,
                                           std::vector<Value *> & reorganizedValues);

    /// @brief 常量声明语句节点翻译
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_const_declare_statement(ast_node * node);

    /// @brief 常量声明节点翻译
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_const_declare(ast_node * node);

    /// @brief 全局常量声明
    /// @param node AST节点
    /// @param typeNode 类型节点
    /// @param nameNode 名称节点
    /// @param initExprNode 初始化表达式节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_global_const_declare(ast_node * node, ast_node * typeNode, ast_node * nameNode, ast_node * initExprNode);

    /// @brief 全局常量标量声明
    /// @param node AST节点
    /// @param typeNode 类型节点
    /// @param nameNode 名称节点
    /// @param initExprNode 初始化表达式节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool
    ir_global_const_scalar_declare(ast_node * node, ast_node * typeNode, ast_node * nameNode, ast_node * initExprNode);

    /// @brief 全局常量数组声明
    /// @param node AST节点
    /// @param typeNode 类型节点
    /// @param nameNode 名称节点
    /// @param initExprNode 初始化表达式节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool
    ir_global_const_array_declare(ast_node * node, ast_node * typeNode, ast_node * nameNode, ast_node * initExprNode);

    /// @brief 局部常量声明
    /// @param node AST节点
    /// @param typeNode 类型节点
    /// @param nameNode 名称节点
    /// @param initExprNode 初始化表达式节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_local_const_declare(ast_node * node, ast_node * typeNode, ast_node * nameNode, ast_node * initExprNode);

    /// @brief 常量折叠
    /// @param value 要折叠的值
    /// @param result 折叠后的结果
    /// @return 翻译是否成功，true：成功，false：失败
    bool try_constant_folding(Value * value, Value *& result);

    /// @brief 常量表达式求值
    /// @param node AST节点
    /// @param result 求值结果
    /// @return 翻译是否成功，true：成功，false：失败
    bool evaluate_const_expr(ast_node * node, Value *& result);

    /// @brief 数组访问AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_array_access(ast_node * node);

    /// @brief 数组初始化AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_array_init(ast_node * node);

    /// @brief 计算数组类型的总基础元素个数（递归计算所有嵌套维度）
    /// @param arrayType 数组类型
    /// @return 总基础元素个数
    int calculateTotalBaseElements(ArrayType * arrayType);

    /// @brief 计算数组类型的总维度数
    /// @param arrayType 数组类型
    /// @return 总维度数
    int calculateTotalDimensions(ArrayType * arrayType);

    /// @brief 递归处理数组初始化，用于全局常量数组
    /// @param initNode 数组初始化节点
    /// @param arrayType 数组类型
    /// @param initValues 输出的初始化值列表
    /// @return 是否成功
    bool processArrayInitialization(ast_node * initNode, ArrayType * arrayType, std::vector<Value *> & initValues);

    /// @brief 数组变量声明和初始化节点翻译成线性中间IR
    /// @param node AST节点
    /// @param typeNode 类型节点
    /// @param varNode 变量名节点
    /// @param initExprNode 数组初始化表达式节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_array_variable_declare_with_init(ast_node * node,
                                             ast_node * typeNode,
                                             ast_node * varNode,
                                             ast_node * initExprNode);

    /// @brief 检测数组初始化是否包含动态值
    /// @param initNode 数组初始化节点
    /// @return true：包含动态值，false：纯静态值
    bool hasRuntimeValues(ast_node * initNode);

    /// @brief 处理零初始化数组
    /// @param node AST节点
    /// @param arrayVar 数组变量
    /// @param arrayType 数组类型
    /// @return 翻译是否成功
    bool handleZeroInitialization(ast_node * node, Value * arrayVar, ArrayType * arrayType);

    /// @brief 处理静态初始化数组
    /// @param node AST节点
    /// @param arrayVar 数组变量
    /// @param arrayType 数组类型
    /// @param initExprNode 初始化表达式节点
    /// @param varName 变量名
    /// @return 翻译是否成功
    bool handleStaticInitialization(ast_node * node,
                                    Value * arrayVar,
                                    ArrayType * arrayType,
                                    ast_node * initExprNode,
                                    const std::string & varName);

    /// @brief 处理动态初始化数组
    /// @param node AST节点
    /// @param arrayVar 数组变量
    /// @param arrayType 数组类型
    /// @param initExprNode 初始化表达式节点
    /// @return 翻译是否成功
    bool handleDynamicInitialization(ast_node * node, Value * arrayVar, ArrayType * arrayType, ast_node * initExprNode);

    /// @brief 处理一维数组动态初始化
    /// @param node AST节点
    /// @param arrayVar 数组变量
    /// @param arrayType 数组类型
    /// @param initExprNode 初始化表达式节点
    /// @return 翻译是否成功
    bool
    handleOneDimensionalDynamicInit(ast_node * node, Value * arrayVar, ArrayType * arrayType, ast_node * initExprNode);
    /// @brief 浮点数加法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_fadd(ast_node * node);

    /// @brief 浮点数减法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_fsub(ast_node * node);

    /// @brief 浮点数乘法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_fmul(ast_node * node);

    /// @brief 浮点数除法AST节点翻译成线性中间IR
    /// @param node AST节点
    /// @return 翻译是否成功，true：成功，false：失败
    bool ir_fdiv(ast_node * node);

    /// @brief 变量信息结构体，用于预收集局部变量
    struct LocalVarInfo {
        std::string name;       // 变量名
        Type * type;            // 变量类型
        uint32_t alignSize;     // 对齐大小
        ast_node * varNode;     // 变量AST节点
        int uniqueId;           // 唯一标识符，用于区分同名变量
        std::string uniqueName; // 唯一名称，如 "a_1", "a_2"

        LocalVarInfo(const std::string & n, Type * t, uint32_t align, ast_node * node, int id)
            : name(n), type(t), alignSize(align), varNode(node), uniqueId(id)
        {
            uniqueName = n + "_" + std::to_string(id);
        }
    };

    /// @brief 预收集函数中的所有局部变量声明
    /// @param node 函数体AST节点
    /// @param localVars 收集到的局部变量列表
    /// @return 收集是否成功
    bool collectLocalVariables(ast_node * node, std::vector<LocalVarInfo> & localVars);

    /// @brief 递归收集局部变量声明的内部实现
    /// @param node AST节点
    /// @param localVars 收集到的局部变量列表
    /// @param varCounter 变量计数器引用
    /// @return 收集是否成功
    bool collectLocalVariablesRecursive(ast_node * node, std::vector<LocalVarInfo> & localVars, int & varCounter);

    /// @brief 在函数开始时生成所有局部变量的alloca指令
    /// @param localVars 局部变量列表
    /// @param blockInsts 指令序列
    /// @return 生成是否成功
    bool generateAllocaInstructions(const std::vector<LocalVarInfo> & localVars, InterCode & blockInsts);

    /// @brief 修改后的变量声明处理，只进行符号表注册
    /// @param node AST节点
    /// @return 翻译是否成功
    bool ir_variable_declare_register_only(ast_node * node);

    /// @brief 将预分配的变量注册到当前作用域
    /// @param name 变量名
    /// @param value 变量值
    /// @return 注册是否成功
    bool registerVariableToCurrentScope(const std::string & name, Value * value);

private:
    /// @brief 当前函数的变量映射表：变量名 -> 预分配的Value
    std::map<std::string, Value *> currentFunctionVarMap;
};
