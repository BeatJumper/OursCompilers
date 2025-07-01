///
/// @file IRGenerator.cpp
/// @brief AST遍历产生线性IR的源文件
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
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>

#include "AST.h"
#include "Common.h"
#include "Function.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Module.h"
#include "ArrayType.h"
#include "LabelInstruction.h"
#include "ExitInstruction.h"
#include "FuncCallInstruction.h"
#include "BinaryInstruction.h"
#include "MoveInstruction.h"
#include "GotoInstruction.h"
#include "RelInstruction.h"
#include "BranchInstruction.h"
#include "AllocaInstruction.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
#include "XorInstruction.h"
#include "ZextInstruction.h"
#include "GetelementptrInstruction.h"
#include "BitcastInstruction.h"
#include "MemcpyInstruction.h"
#include "MemsetInstruction.h"
#include "SextInstruction.h"
#include "SitofpInstruction.h"
#include "FptosiInstruction.h"
#include "PointerType.h"
#include "ConstFloat.h"
#include "DeadCodeElimination.h"

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module) : root(_root), module(_module)
{
    /* 叶子节点 */
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT] = &IRGenerator::ir_leaf_node_float;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

    // 常量声明
    ast2ir_handlers[ast_operator_type::AST_OP_CONST_DECL_STMT] = &IRGenerator::ir_const_declare_statement;
    ast2ir_handlers[ast_operator_type::AST_OP_CONST_DECL] = &IRGenerator::ir_const_declare;

    /* 表达式运算， 加减 */
    ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add_or_fadd;
    ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub_or_fsub;
    ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul_or_fmul;
    ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div_or_fdiv;

    ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod;
    ast2ir_handlers[ast_operator_type::AST_OP_POSITIVE] = &IRGenerator::ir_positive;
    ast2ir_handlers[ast_operator_type::AST_OP_NEGATIVE] = &IRGenerator::ir_negative;
    ast2ir_handlers[ast_operator_type::AST_OP_NOT] = &IRGenerator::ir_not;
    ast2ir_handlers[ast_operator_type::AST_OP_AND] = &IRGenerator::ir_and;
    ast2ir_handlers[ast_operator_type::AST_OP_OR] = &IRGenerator::ir_or;

    /* 关系表达式运算 */
    ast2ir_handlers[ast_operator_type::AST_OP_LT] = &IRGenerator::ir_rel_exp;
    ast2ir_handlers[ast_operator_type::AST_OP_GT] = &IRGenerator::ir_rel_exp;
    ast2ir_handlers[ast_operator_type::AST_OP_LE] = &IRGenerator::ir_rel_exp;
    ast2ir_handlers[ast_operator_type::AST_OP_GE] = &IRGenerator::ir_rel_exp;
    ast2ir_handlers[ast_operator_type::AST_OP_EQ] = &IRGenerator::ir_rel_exp;
    ast2ir_handlers[ast_operator_type::AST_OP_NE] = &IRGenerator::ir_rel_exp;

    /* 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;

    /* 函数调用 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

    /* 函数定义 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

    /* 变量定义语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_DECL_STMT] = &IRGenerator::ir_declare_statment;
    ast2ir_handlers[ast_operator_type::AST_OP_VAR_DECL] = &IRGenerator::ir_variable_declare;

    /* while 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while;

    /* if-else 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if_else;

    /* break, continune */
    ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break;
    ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue;

    /* 语句块 */
    ast2ir_handlers[ast_operator_type::AST_OP_BLOCK] = &IRGenerator::ir_block;

    // 数组相关处理
    ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_ACCESS] = &IRGenerator::ir_array_access;
    ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_INIT] = &IRGenerator::ir_array_init;

    /* 编译单元 */
    ast2ir_handlers[ast_operator_type::AST_OP_COMPILE_UNIT] = &IRGenerator::ir_compile_unit;
}

/// @brief 遍历抽象语法树产生线性IR，保存到IRCode中
/// @param root 抽象语法树
/// @param IRCode 线性IR
/// @return true: 成功 false: 失败
bool IRGenerator::run()
{
    ast_node * node;

    // 从根节点进行遍历
    node = ir_visit_ast_node(root);

    if (node) {
        printf("Debug: Successfully processed root node.\n");
    } else {
        printf("Debug: Failed to process root node.\n");
    }

    return node != nullptr;
}

/// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
/// @param node AST节点
/// @return 成功返回node节点，否则返回nullptr
ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
{
    // 空节点
    if (nullptr == node) {
        return nullptr;
    }

    bool result;

    // 根据节点类型查找对应的翻译函数
    std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pIter;
    pIter = ast2ir_handlers.find(node->node_type);
    if (pIter == ast2ir_handlers.end()) {
        // 没有找到，则说明当前不支持
        result = (this->ir_default)(node);
    } else {
        result = (this->*(pIter->second))(node);
    }

    if (!result) {
        // 语义解析错误，则出错返回
        node = nullptr;
    }

    return node;
}

/// @brief 未知节点类型的节点处理
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_default(ast_node * node)
{
    // 未知的节点
    printf("Unkown node(%d)\n", (int) node->node_type);
    return true;
}

/// @brief 编译单元AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_compile_unit(ast_node * node)
{
    module->setCurrentFunction(nullptr);

    for (auto son: node->sons) {

        // 遍历编译单元，要么是函数定义，要么是语句
        ast_node * son_node = ir_visit_ast_node(son);
        if (!son_node) {
            // TODO 自行追加语义错误处理
            return false;
        }
    }

    return true;
}

/// @brief 函数定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_define(ast_node * node)
{
    // printf("==== ENTER ir_function_define ====\n");

    // 检查是否有嵌套函数定义（不允许）
    if (module->getCurrentFunction()) {
        printf("Error: Nested function definition is not allowed.\n");
        return false;
    }

    // 函数定义的AST包含四个孩子
    // 第一个孩子：函数返回类型
    // 第二个孩子：函数名字
    // 第三个孩子：形参列表
    // 第四个孩子：函数体即block
    ast_node * type_node = node->sons[0];
    ast_node * name_node = node->sons[1];
    ast_node * param_node = node->sons[2];
    ast_node * block_node = node->sons[3];

    // 创建新函数
    Function * newFunc = module->newFunction(name_node->name, type_node->type);
    if (!newFunc) {
        printf("Error: Function %s already exists.\n", name_node->name.c_str());
        return false;
    }

    // 设置当前函数并进入新作用域
    module->setCurrentFunction(newFunc);
    module->enterScope();
    InterCode & irCode = newFunc->getInterCode();

    // 创建函数入口标签
    LabelInstruction * entryLabelInst = new LabelInstruction(newFunc, module);
    irCode.addInst(entryLabelInst);

    // 创建函数出口标签（稍后添加）
    LabelInstruction * exitLabelInst = new LabelInstruction(newFunc, module);
    newFunc->setExitLabel(exitLabelInst);

    // 处理函数形参
    if (!ir_function_formal_params(param_node)) {
        printf("Error: Failed to process function parameters.\n");
        return false;
    }
    irCode.addInst(param_node->blockInsts);

    // 为非void函数创建返回值变量
    LocalVariable * retValue = nullptr;
    if (!type_node->type->isVoidType()) {
        Type * retType = type_node->type;             // 返回值类型
        Type * retVarType = new PointerType(retType); // 返回值变量类型（指针类型）
        retValue = static_cast<LocalVariable *>(module->newVarValue(retVarType, "__ret"));
        AllocaInstruction * allocaRet = new AllocaInstruction(newFunc, retValue, retType, 4);
        irCode.addInst(allocaRet);

        // 只有main函数初始化返回值为0
        if (name_node->name == "main") {
            Value * zeroConst = nullptr;
            if (type_node->type->isFloatType()) {
                zeroConst = module->newConstFloat(0.0f);
            } else {
                zeroConst = module->newConstInt(0);
            }
            StoreInstruction * storeRet = new StoreInstruction(newFunc, zeroConst, retValue, 4);
            irCode.addInst(storeRet);
        }
    }
    newFunc->setReturnValue(retValue);

    // 预收集函数体中的所有局部变量
    // 预扫描所有作用域，收集所有变量声明（包括同名变量）
    std::vector<LocalVarInfo> localVars;
    if (!collectLocalVariables(block_node, localVars)) {
        printf("Error: Failed to collect local variables in function '%s'.\n", name_node->name.c_str());
        return false;
    }

    // 在函数开始时生成所有局部变量的alloca指令
    if (!generateAllocaInstructions(localVars, irCode)) {
        printf("Error: Failed to generate alloca instructions in function '%s'.\n", name_node->name.c_str());
        return false;
    }

    // printf("Debug: Generated %zu alloca instructions for function '%s'\n", localVars.size(),
    // name_node->name.c_str());

    // 处理函数体（不需要新的作用域，因为函数本身就是一个作用域）
    block_node->needScope = false;
    if (!ir_block(block_node)) {
        printf("Error: Failed to process function body.\n");
        return false;
    }

    // 将函数体的指令直接添加到函数的IR中
    irCode.addInst(block_node->blockInsts);

    // 检查函数体的最后一条指令是否是终结指令
    bool needsGotoToExit = true;
    if (!irCode.getCode().empty()) {
        Instruction * lastInst = irCode.getCode().back();
        IRInstOperator op = lastInst->getOp();
        if (op == IRInstOperator::IRINST_OP_GOTO || op == IRInstOperator::IRINST_OP_RET ||
            op == IRInstOperator::IRINST_OP_BRANCH) {
            needsGotoToExit = false;
        }
    }

    if (needsGotoToExit) {
        // 添加跳转到出口标签的指令
        irCode.addInst(new GotoInstruction(newFunc, exitLabelInst));
    }

    // 总是添加函数出口标签（return语句可能会跳转到这里）
    irCode.addInst(exitLabelInst);

    // 添加函数返回指令
    if (!type_node->type->isVoidType() && retValue) {
        // 非void函数：加载返回值并返回
        LoadInstruction * loadRet = new LoadInstruction(newFunc, retValue, retValue, 4);
        irCode.addInst(loadRet);
        irCode.addInst(new ExitInstruction(newFunc, loadRet));
    } else {
        // void函数：直接返回
        irCode.addInst(new ExitInstruction(newFunc, nullptr));
    }

    // 死代码消除优化
    DeadCodeElimination dce;
    bool optimized = dce.eliminateDeadCode(newFunc);
    if (optimized) {
        // printf("Dead code elimination applied to function %s\n", name_node->name.c_str());
    }

    // 恢复外部状态
    module->setCurrentFunction(nullptr);
    module->leaveScope();

    // printf("==== EXIT ir_function_define ====\n");
    // printf("Function has %zu instructions\n", irCode.getInsts().size());
    std::string fullIR;
    newFunc->toString(fullIR);
    // printf("Final IR after rename:\n%s\n", fullIR.c_str());
    return true;
}

/// @brief 形式参数AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
    // TODO 目前形参还不支持，直接返回true

    // 每个形参变量都创建对应的临时变量，用于表达实参转递的值
    // 而真实的形参则创建函数内的局部变量。
    // 然后产生赋值指令，用于把表达实参值的临时变量拷贝到形参局部变量上。
    // 请注意这些指令要放在Entry指令后面，因此处理的先后上要注意。

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        printf("Error: No current function in ir_function_formal_params.\n");
        return false;
    }

    // 记录形参和局部变量的配对
    std::vector<std::pair<FormalParam *, LocalVariable *>> paramPairs;

    // 遍历形参列表
    for (auto & paramNode: node->sons) {
        // 每个形参节点应该有两个子节点：类型节点和变量名节点
        if (paramNode->sons.size() != 2) {
            printf("Error: Invalid parameter node structure in ir_function_formal_params.\n");
            return false;
        }

        ast_node * typeNode = paramNode->sons[0]; // 类型节点
        ast_node * nameNode = paramNode->sons[1]; // 变量名节点

        // 创建一个形参对象，表示函数参数
        FormalParam * param = new FormalParam(typeNode->type, nameNode->name);

        // 将形参添加到函数的形参列表中
        currentFunc->getParams().push_back(param);

        // 创建一个局部变量表示在函数体内使用的参数
        Type * paramType = typeNode->type;
        Type * allocaType;   // 要分配的类型
        Type * localVarType; // 局部变量的类型

        if (paramType->isPointerType()) {
            // 数组参数：参数类型是i32*，需要分配i32*类型的空间，局部变量类型是i32**
            allocaType = paramType;                    // 分配i32*类型的空间
            localVarType = new PointerType(paramType); // 局部变量类型是i32**
        } else {
            // 普通参数：参数类型是i32，需要分配i32类型的空间，局部变量类型是i32*
            allocaType = paramType;                    // 分配i32类型的空间
            localVarType = new PointerType(paramType); // 局部变量类型是i32*
        }

        Value * paramVar = module->newVarValue(localVarType, nameNode->name);

        if (!paramVar) {
            printf("Error: Failed to create local variable for parameter '%s'.\n", nameNode->name.c_str());
            return false;
        }

        // 转换为 LocalVariable 类型
        LocalVariable * localParamVar = static_cast<LocalVariable *>(paramVar);

        // 创建 alloca 指令，分配参数类型的空间
        uint32_t alignSize = allocaType->getSize();
        AllocaInstruction * allocaInst = new AllocaInstruction(currentFunc, paramVar, allocaType, alignSize);
        currentFunc->getInterCode().addInst(allocaInst);

        // 设置形参节点的值为创建的局部变量（函数体内使用这个变量）
        nameNode->val = paramVar;

        // 将 (形参, 局部变量) 对添加到列表中
        paramPairs.emplace_back(param, localParamVar);
    }

    // 再store所有形参
    for (auto & pair: paramPairs) {
        // 使用与参数类型匹配的对齐
        uint32_t alignSize = pair.second->getType()->getSize();
        StoreInstruction * storeInst = new StoreInstruction(currentFunc, pair.first, pair.second, alignSize);
        currentFunc->getInterCode().addInst(storeInst);
    }

    return true;
}

/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_call(ast_node * node)
{
    std::vector<Value *> loadedParams;

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();

    // 函数调用的节点包含两个节点：
    // 第一个节点：函数名节点
    // 第二个节点：实参列表节点
    std::string funcName = node->sons[0]->name;
    int64_t lineno = node->sons[0]->line_no;

    ast_node * paramsNode = node->sons[1];

    // 根据函数名查找函数，看是否存在。若不存在则出错
    auto calledFunction = module->findFunction(funcName);
    if (nullptr == calledFunction) {
        printf("Error: Function '%s' not found at line %ld.\n", funcName.c_str(), lineno);
        return false;
    }

    // 当前函数存在函数调用
    currentFunc->setExistFuncCall(true);

    // 处理参数列表（只处理一次）
    if (!paramsNode->sons.empty()) {
        int32_t argsCount = (int32_t) paramsNode->sons.size();

        // 当前函数中调用函数实参个数最大值统计
        if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
            currentFunc->setMaxFuncCallArgCnt(argsCount);
        }

        // 遍历参数列表
        for (auto son: paramsNode->sons) {
            // 计算参数表达式
            ast_node * temp = ir_visit_ast_node(son);
            if (!temp) {
                return false;
            }

            // 添加参数表达式的指令
            node->blockInsts.addInst(temp->blockInsts);

            Value * paramValue = nullptr;

            // 检查参数类型，处理数组到指针的转换
            if (temp->val->getType()->isArrayType()) {
                // 直接的数组类型：需要获取数组的首地址
                // 使用 getelementptr 获取数组首元素地址
                ConstInt * zeroConst = module->newConstInt(0);
                ConstInt * zeroConst2 = module->newConstInt(0);

                GetelementptrInstruction * gepInst =
                    new GetelementptrInstruction(currentFunc, temp->val, zeroConst, zeroConst2);
                node->blockInsts.addInst(gepInst);
                paramValue = gepInst;
            } else if (temp->val->getType()->isPointerType()) {
                // 检查是否是指向数组的指针（如数组访问的结果）
                const PointerType * ptrType = static_cast<const PointerType *>(temp->val->getType());
                if (ptrType->getPointeeType()->isArrayType()) {
                    // 指向数组的指针：需要转换为指向元素的指针
                    // 使用 getelementptr 获取数组首元素地址
                    ConstInt * zeroConst = module->newConstInt(0);
                    ConstInt * zeroConst2 = module->newConstInt(0);

                    GetelementptrInstruction * gepInst =
                        new GetelementptrInstruction(currentFunc, temp->val, zeroConst, zeroConst2);
                    node->blockInsts.addInst(gepInst);
                    paramValue = gepInst;
                } else if (needsLoad(temp->val)) {
                    // 普通变量（如局部变量），需要加载值
                    LoadInstruction * loadParam = new LoadInstruction(currentFunc, temp->val, temp->val, 4);
                    node->blockInsts.addInst(loadParam);
                    paramValue = loadParam;
                } else {
                    // 其他指针类型（如函数参数），直接使用
                    paramValue = temp->val;
                }
            } else if (needsLoad(temp->val)) {
                // 参数是变量，需要加载
                LoadInstruction * loadParam = new LoadInstruction(currentFunc, temp->val, temp->val, 4);
                node->blockInsts.addInst(loadParam);
                paramValue = loadParam;
            } else {
                // 参数是常量或表达式结果，直接使用
                paramValue = temp->val;
            }

            // 参数类型转换：确保参数类型与函数期望的类型匹配
            if (loadedParams.size() < calledFunction->getParams().size()) {
                Type * expectedType = calledFunction->getParams()[loadedParams.size()]->getType();

                // 如果期望的是浮点数类型，但传入的是整数类型，进行转换
                if (expectedType->isFloatType() && paramValue->getType()->isIntegerType()) {
                    paramValue = convertToFloat(paramValue, currentFunc, node->blockInsts);
                    if (!paramValue) {
                        printf("Error: Failed to convert parameter to float type in function call.\n");
                        return false;
                    }
                }
                // 如果期望的是整数类型，但传入的是浮点数类型，进行转换
                else if (expectedType->isIntegerType() && paramValue->getType()->isFloatType()) {
                    paramValue = convertToInt(paramValue, currentFunc, node->blockInsts);
                    if (!paramValue) {
                        printf("Error: Failed to convert parameter to integer type in function call.\n");
                        return false;
                    }
                }
            }

            // 将参数值添加到参数列表
            loadedParams.push_back(paramValue);
        }
    }

    // 参数个数检查
    if (loadedParams.size() != calledFunction->getParams().size()) {
        printf("Error: Function '%s' parameter count mismatch at line %ld. Expected %zu, got %zu.\n",
               funcName.c_str(),
               lineno,
               calledFunction->getParams().size(),
               loadedParams.size());
        return false;
    }

    // 函数返回类型
    Type * returnType = calledFunction->getReturnType();

    // 关键修复：使用正确的构造函数
    // 不要使用带有vector<Value*>参数的构造函数，而是使用基本构造函数然后添加操作数
    FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, loadedParams, returnType);

    // 添加函数调用指令
    node->blockInsts.addInst(funcCallInst);

    // 函数调用结果保存到node中
    node->val = funcCallInst;

    return true;
}

/// @brief 语句块（含函数体）AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_block(ast_node * node)
{
    // 进入作用域
    if (node->needScope) {
        module->enterScope();
    }

    std::vector<ast_node *>::iterator pIter;
    for (pIter = node->sons.begin(); pIter != node->sons.end(); ++pIter) {

        // 遍历Block的每个语句，进行显示或者运算
        ast_node * temp = ir_visit_ast_node(*pIter);
        if (!temp) {
            return false;
        }

        node->blockInsts.addInst(temp->blockInsts);

        // 检查当前语句是否包含return指令
        // 只有return指令才会导致后续语句为不可达代码
        // break和continue不应该阻止同一语句块中后续语句的处理
        if (hasReturnInstruction(temp->blockInsts)) {
            // 跳出循环，不再处理后续语句
            break;
        }
    }

    // 离开作用域
    if (node->needScope) {
        module->leaveScope();
    }

    return true;
}

bool IRGenerator::ir_add_or_fadd(ast_node * node)
{
    // 先递归处理左右操作数，获取它们的值类型
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right || !left->val || !right->val) {
        // printf("Debug: Failed to process operands in ir_add_or_fadd\n");
        return false;
    }

    // 常量折叠检查：如果两个操作数都是常量，直接计算结果
    ConstInt * leftConstInt = dynamic_cast<ConstInt *>(left->val);
    ConstInt * rightConstInt = dynamic_cast<ConstInt *>(right->val);
    ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(left->val);
    ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(right->val);

    if ((leftConstInt || leftConstFloat) && (rightConstInt || rightConstFloat)) {
        // 常量折叠：两个操作数都是常量
        // printf("Debug: Performing constant folding for addition\n");

        // 如果任一操作数是浮点数，结果为浮点数
        if (leftConstFloat || rightConstFloat) {
            float leftVal = leftConstFloat ? leftConstFloat->getVal() : static_cast<float>(leftConstInt->getVal());
            float rightVal = rightConstFloat ? rightConstFloat->getVal() : static_cast<float>(rightConstInt->getVal());
            float result = leftVal + rightVal;

            ConstFloat * resultConst = module->newConstFloat(result);
            node->val = resultConst;
            // printf("Debug: Constant folding result: %f + %f = %f\n", leftVal, rightVal, result);
            return true;
        } else {
            // 两个都是整数常量
            int32_t leftVal = leftConstInt->getVal();
            int32_t rightVal = rightConstInt->getVal();
            int32_t result = leftVal + rightVal;

            ConstInt * resultConst = module->newConstInt(result);
            node->val = resultConst;
            // printf("Debug: Constant folding result: %d + %d = %d\n", leftVal, rightVal, result);
            return true;
        }
    }

    // 检查操作数的值类型（如果是指针类型，获取指向的类型）
    Type * leftType = left->val->getType();
    Type * rightType = right->val->getType();

    // 如果是指针类型，获取指向的类型（用于数组元素）
    if (leftType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(leftType);
        leftType = const_cast<Type *>(ptrType->getPointeeType());
    }
    if (rightType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(rightType);
        rightType = const_cast<Type *>(ptrType->getPointeeType());
    }

    // 如果任一操作数是浮点类型，使用浮点加法
    if (leftType->isFloatType() || rightType->isFloatType()) {
        // 将已处理的子节点信息复制到当前节点
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        return ir_fadd_processed(node, left, right); // 使用已处理的操作数
    } else {
        // 将已处理的子节点信息复制到当前节点
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        return ir_add_processed(node, left, right); // 使用已处理的操作数
    }
}

bool IRGenerator::ir_sub_or_fsub(ast_node * node)
{
    // 先递归处理左右操作数，获取它们的值类型
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right || !left->val || !right->val) {
        return false;
    }

    // 常量折叠检查：如果两个操作数都是常量，直接计算结果
    ConstInt * leftConstInt = dynamic_cast<ConstInt *>(left->val);
    ConstInt * rightConstInt = dynamic_cast<ConstInt *>(right->val);
    ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(left->val);
    ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(right->val);

    if ((leftConstInt || leftConstFloat) && (rightConstInt || rightConstFloat)) {
        // 常量折叠：两个操作数都是常量
        // printf("Debug: Performing constant folding for subtraction\n");

        // 如果任一操作数是浮点数，结果为浮点数
        if (leftConstFloat || rightConstFloat) {
            float leftVal = leftConstFloat ? leftConstFloat->getVal() : static_cast<float>(leftConstInt->getVal());
            float rightVal = rightConstFloat ? rightConstFloat->getVal() : static_cast<float>(rightConstInt->getVal());
            float result = leftVal - rightVal;

            ConstFloat * resultConst = module->newConstFloat(result);
            node->val = resultConst;
            // printf("Debug: Constant folding result: %f - %f = %f\n", leftVal, rightVal, result);
            return true;
        } else {
            // 两个都是整数常量
            int32_t leftVal = leftConstInt->getVal();
            int32_t rightVal = rightConstInt->getVal();
            int32_t result = leftVal - rightVal;

            ConstInt * resultConst = module->newConstInt(result);
            node->val = resultConst;
            // printf("Debug: Constant folding result: %d - %d = %d\n", leftVal, rightVal, result);
            return true;
        }
    }

    // 检查操作数的值类型（如果是指针类型，获取指向的类型）
    Type * leftType = left->val->getType();
    Type * rightType = right->val->getType();

    // 如果是指针类型，获取指向的类型（用于数组元素）
    if (leftType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(leftType);
        leftType = const_cast<Type *>(ptrType->getPointeeType());
    }
    if (rightType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(rightType);
        rightType = const_cast<Type *>(ptrType->getPointeeType());
    }

    // 如果任一操作数是浮点类型，使用浮点减法
    if (leftType->isFloatType() || rightType->isFloatType()) {
        // 将已处理的子节点信息复制到当前节点
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        return ir_fsub_processed(node, left, right);
    } else {
        // 将已处理的子节点信息复制到当前节点
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        return ir_sub_processed(node, left, right);
    }
}

bool IRGenerator::ir_mul_or_fmul(ast_node * node)
{
    // 先递归处理左右操作数，获取它们的值类型
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right || !left->val || !right->val) {
        return false;
    }

    // 常量折叠检查：如果两个操作数都是常量，直接计算结果
    ConstInt * leftConstInt = dynamic_cast<ConstInt *>(left->val);
    ConstInt * rightConstInt = dynamic_cast<ConstInt *>(right->val);
    ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(left->val);
    ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(right->val);

    if ((leftConstInt || leftConstFloat) && (rightConstInt || rightConstFloat)) {
        // 常量折叠：两个操作数都是常量
        // printf("Debug: Performing constant folding for multiplication\n");

        // 如果任一操作数是浮点数，结果为浮点数
        if (leftConstFloat || rightConstFloat) {
            float leftVal = leftConstFloat ? leftConstFloat->getVal() : static_cast<float>(leftConstInt->getVal());
            float rightVal = rightConstFloat ? rightConstFloat->getVal() : static_cast<float>(rightConstInt->getVal());
            float result = leftVal * rightVal;

            ConstFloat * resultConst = module->newConstFloat(result);
            node->val = resultConst;
            // printf("Debug: Constant folding result: %f * %f = %f\n", leftVal, rightVal, result);
            return true;
        } else {
            // 两个都是整数常量
            int32_t leftVal = leftConstInt->getVal();
            int32_t rightVal = rightConstInt->getVal();
            int32_t result = leftVal * rightVal;

            ConstInt * resultConst = module->newConstInt(result);
            node->val = resultConst;
            // printf("Debug: Constant folding result: %d * %d = %d\n", leftVal, rightVal, result);
            return true;
        }
    }

    // 检查操作数的值类型（如果是指针类型，获取指向的类型）
    Type * leftType = left->val->getType();
    Type * rightType = right->val->getType();

    // 如果是指针类型，获取指向的类型（用于数组元素）
    if (leftType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(leftType);
        leftType = const_cast<Type *>(ptrType->getPointeeType());
    }
    if (rightType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(rightType);
        rightType = const_cast<Type *>(ptrType->getPointeeType());
    }

    // 如果任一操作数是浮点类型，使用浮点乘法
    if (leftType->isFloatType() || rightType->isFloatType()) {
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        return ir_fmul_processed(node, left, right);
    } else {
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        return ir_mul_processed(node, left, right);
    }
}

bool IRGenerator::ir_div_or_fdiv(ast_node * node)
{
    // 先递归处理左右操作数，获取它们的值类型
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right || !left->val || !right->val) {
        return false;
    }

    // 常量折叠检查：如果两个操作数都是常量，直接计算结果
    ConstInt * leftConstInt = dynamic_cast<ConstInt *>(left->val);
    ConstInt * rightConstInt = dynamic_cast<ConstInt *>(right->val);
    ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(left->val);
    ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(right->val);

    if ((leftConstInt || leftConstFloat) && (rightConstInt || rightConstFloat)) {
        // 常量折叠：两个操作数都是常量
        // printf("Debug: Performing constant folding for division\n");

        // 检查除零错误
        if ((rightConstInt && rightConstInt->getVal() == 0) || (rightConstFloat && rightConstFloat->getVal() == 0.0f)) {
            printf("Error: Division by zero in constant folding\n");
            return false;
        }

        // 如果任一操作数是浮点数，结果为浮点数
        if (leftConstFloat || rightConstFloat) {
            float leftVal = leftConstFloat ? leftConstFloat->getVal() : static_cast<float>(leftConstInt->getVal());
            float rightVal = rightConstFloat ? rightConstFloat->getVal() : static_cast<float>(rightConstInt->getVal());
            float result = leftVal / rightVal;

            ConstFloat * resultConst = module->newConstFloat(result);
            node->val = resultConst;
            // printf("Debug: Constant folding result: %f / %f = %f\n", leftVal, rightVal, result);
            return true;
        } else {
            // 两个都是整数常量
            int32_t leftVal = leftConstInt->getVal();
            int32_t rightVal = rightConstInt->getVal();
            int32_t result = leftVal / rightVal;

            ConstInt * resultConst = module->newConstInt(result);
            node->val = resultConst;
            // printf("Debug: Constant folding result: %d / %d = %d\n", leftVal, rightVal, result);
            return true;
        }
    }

    // 检查操作数的值类型（如果是指针类型，获取指向的类型）
    Type * leftType = left->val->getType();
    Type * rightType = right->val->getType();

    // 如果是指针类型，获取指向的类型（用于数组元素）
    if (leftType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(leftType);
        leftType = const_cast<Type *>(ptrType->getPointeeType());
    }
    if (rightType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(rightType);
        rightType = const_cast<Type *>(ptrType->getPointeeType());
    }

    // 如果任一操作数是浮点类型，使用浮点除法
    if (leftType->isFloatType() || rightType->isFloatType()) {
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        return ir_fdiv_processed(node, left, right);
    } else {
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        return ir_div_processed(node, left, right);
    }
}

/// @brief 整数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 生成加法指令
    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(addInst);
    node->val = addInst;

    return true;
}

/// @brief 整数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 生成加法指令
    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(addInst);
    node->val = addInst;

    return true;
}

/// @brief 整数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mul(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 生成加法指令
    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(addInst);
    node->val = addInst;

    return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_div(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 生成加法指令
    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(addInst);
    node->val = addInst;

    return true;
}

/// @brief 整数加法AST节点翻译成线性中间IR（使用已处理的操作数）
/// @param node AST节点
/// @param left 已处理的左操作数节点
/// @param right 已处理的右操作数节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add_processed(ast_node * node, ast_node * left, ast_node * right)
{
    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量或数组访问需要load
        int loadSize = 4; // 默认4字节
        if (left->val->getType()->isPointerType()) {
            const PointerType * ptrType = static_cast<const PointerType *>(left->val->getType());
            const Type * pointeeType = ptrType->getPointeeType();
            if (pointeeType->isFloatType()) {
                loadSize = 4; // float 类型使用 4 字节对齐
            } else if (pointeeType->isIntegerType()) {
                loadSize = 4; // int 类型使用 4 字节对齐
            } else {
                loadSize = 4; // 其他类型也使用 4 字节对齐
            }
        }
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, loadSize);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        // 变量或数组访问需要load
        int loadSize = 4; // 默认4字节
        if (right->val->getType()->isPointerType()) {
            const PointerType * ptrType = static_cast<const PointerType *>(right->val->getType());
            const Type * pointeeType = ptrType->getPointeeType();
            if (pointeeType->isFloatType()) {
                loadSize = 4; // float 类型使用 4 字节对齐
            } else if (pointeeType->isIntegerType()) {
                loadSize = 4; // int 类型使用 4 字节对齐
            } else {
                loadSize = 4; // 其他类型也使用 4 字节对齐
            }
        }
        LoadInstruction * loadRight =
            new LoadInstruction(module->getCurrentFunction(), right->val, right->val, loadSize);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 处理类型转换
    if (!handleArithmeticTypeConversion(node, leftValue, rightValue)) {
        return false;
    }

    // 生成加法指令
    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(addInst);
    node->val = addInst;

    return true;
}

/// @brief 浮点数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_fadd(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 确保操作数是浮点类型
    if (!leftValue->getType()->isFloatType()) {
        leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
        if (!leftValue)
            return false;
    }
    if (!rightValue->getType()->isFloatType()) {
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue)
            return false;
    }

    // 生成浮点加法指令
    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_F,
                                                        leftValue,
                                                        rightValue,
                                                        FloatType::getTypeFloat());
    node->blockInsts.addInst(addInst);
    node->val = addInst;

    return true;
}

/// @brief 浮点数加法AST节点翻译成线性中间IR（使用已处理的操作数）
/// @param node AST节点
/// @param left 已处理的左操作数节点
/// @param right 已处理的右操作数节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_fadd_processed(ast_node * node, ast_node * left, ast_node * right)
{
    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 确保操作数是浮点类型
    if (!leftValue->getType()->isFloatType()) {
        leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
        if (!leftValue)
            return false;
    }
    if (!rightValue->getType()->isFloatType()) {
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue)
            return false;
    }

    // 生成浮点加法指令
    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_F,
                                                        leftValue,
                                                        rightValue,
                                                        FloatType::getTypeFloat());
    node->blockInsts.addInst(addInst);
    node->val = addInst;

    return true;
}

/// @brief 浮点数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_fsub(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 确保操作数是浮点类型
    if (!leftValue->getType()->isFloatType()) {
        leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
        if (!leftValue)
            return false;
    }
    if (!rightValue->getType()->isFloatType()) {
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue)
            return false;
    }

    // 生成浮点减法指令
    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_F,
                                                        leftValue,
                                                        rightValue,
                                                        FloatType::getTypeFloat());
    node->blockInsts.addInst(subInst);
    node->val = subInst;

    return true;
}

/// @brief 浮点数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_fmul(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 确保操作数是浮点类型
    if (!leftValue->getType()->isFloatType()) {
        leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
        if (!leftValue)
            return false;
    }
    if (!rightValue->getType()->isFloatType()) {
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue)
            return false;
    }

    // 生成浮点乘法指令
    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_F,
                                                        leftValue,
                                                        rightValue,
                                                        FloatType::getTypeFloat());
    node->blockInsts.addInst(mulInst);
    node->val = mulInst;

    return true;
}

/// @brief 浮点数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_fdiv(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        // 变量需要load
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 确保操作数是浮点类型
    if (!leftValue->getType()->isFloatType()) {
        leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
        if (!leftValue)
            return false;
    }
    if (!rightValue->getType()->isFloatType()) {
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue)
            return false;
    }

    // 生成浮点除法指令
    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_F,
                                                        leftValue,
                                                        rightValue,
                                                        FloatType::getTypeFloat());
    node->blockInsts.addInst(divInst);
    node->val = divInst;

    return true;
}

/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 赋值运算符的左侧操作数
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left || !left->val) {
        printf("Error: Left operand has no Value in ir_assign.\n");
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right || !right->val) {
        printf("Error: Right operand has no Value in ir_assign.\n");
        return false;
    }

    Value * rightValue = nullptr;

    // 处理右操作数
    // 修复：对于数组访问表达式，右侧的值应该是从地址加载的值
    if (son2_node->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
        // 右操作数是数组访问，right->val 是地址，需要加载值
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(right->blockInsts);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else if (needsLoad(right->val)) {
        // 右操作数是变量，需要加载
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(right->blockInsts);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        // 右操作数是常量或表达式结果，直接使用
        node->blockInsts.addInst(right->blockInsts);
        rightValue = right->val;
    }

    // 检查 rightValue 的类型
    if (rightValue->getType()->isPointerType()) {
        printf("Error: Attempting to store pointer value instead of actual value in ir_assign.\n");
        // printf("Debug: Right operand type: %s\n", rightValue->getType()->toString().c_str());
        // printf("Debug: Right operand IR name: %s\n", rightValue->getIRName().c_str());
        return false;
    }

    // 获取目标类型（如果是指针类型，获取指向的类型）
    Type * targetType = left->val->getType();
    if (targetType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(targetType);
        targetType = const_cast<Type *>(ptrType->getPointeeType());
    }

    // 类型转换检查
    if (targetType->isFloatType() && !rightValue->getType()->isFloatType()) {
        // 目标类型是浮点数，源类型不是，需要转换
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue) {
            printf("Error: Failed to convert right operand to float type in ir_assign.\n");
            return false;
        }
    } else if (targetType->isIntegerType() && !rightValue->getType()->isIntegerType()) {
        // 目标类型是整数，源类型不是，需要转换
        rightValue = convertToInt(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue) {
            printf("Error: Failed to convert right operand to integer type in ir_assign.\n");
            return false;
        }
    }

    // 创建 store 指令，将右侧值存储到左侧地址
    StoreInstruction * storeInst = new StoreInstruction(module->getCurrentFunction(), rightValue, left->val, 4);

    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(storeInst);

    node->val = rightValue;

    return true;
}

Value * IRGenerator::convertToFloat(Value * val, Function * func, InterCode & blockInsts)
{
    if (val->getType()->isFloatType()) {
        return val; // 已经是浮点数，无需转换
    }

    if (val->getType()->isIntegerType()) {
        // 整数转浮点数 - 使用 sitofp 指令
        Instruction * convInst = new SitofpInstruction(func, val, FloatType::getTypeFloat());
        blockInsts.addInst(convInst);
        return convInst;
    }

    printf("Error: Cannot convert value to float type\n");
    return nullptr;
}

Value * IRGenerator::convertToInt(Value * val, Function * func, InterCode & blockInsts)
{
    if (val->getType()->isIntegerType()) {
        return val; // 已经是整数，无需转换
    }

    if (val->getType()->isFloatType()) {
        // 浮点数转整数 - 使用 fptosi 指令
        Instruction * convInst = new FptosiInstruction(func, val, IntegerType::getTypeInt());
        blockInsts.addInst(convInst);
        return convInst;
    }

    printf("Error: Cannot convert value to integer type\n");
    return nullptr;
}
/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
    // 获取当前函数
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        printf("Error: Return statement outside function.\n");
        return false;
    }

    // 获取函数返回类型
    Type * returnType = currentFunc->getReturnType();
    Value * returnValue = nullptr;
    ast_node * right = nullptr;

    // 处理return语句的表达式（如果有）
    if (!node->sons.empty()) {
        ast_node * son_node = node->sons[0];
        right = ir_visit_ast_node(son_node);
        if (!right) {
            printf("Error: Failed to evaluate return expression.\n");
            return false;
        }
        node->blockInsts.addInst(right->blockInsts);

        // 处理返回值
        if (right->val) {
            // 检查是否需要加载返回值
            if (needsLoad(right->val)) {
                // 对于需要加载的值，使用固定的对齐方式
                int alignment = 4; // 默认对齐方式

                // 如果是指针类型，获取指向的类型来确定对齐方式
                if (right->val->getType()->isPointerType()) {
                    const PointerType * ptrType = static_cast<const PointerType *>(right->val->getType());
                    const Type * pointeeType = ptrType->getPointeeType();
                    if (pointeeType->isFloatType()) {
                        alignment = 4; // float 类型使用 4 字节对齐
                    } else if (pointeeType->isIntegerType()) {
                        alignment = 4; // int 类型使用 4 字节对齐
                    } else {
                        alignment = 4; // 其他类型也使用 4 字节对齐
                    }
                }

                LoadInstruction * loadRight = new LoadInstruction(currentFunc, right->val, right->val, alignment);
                node->blockInsts.addInst(loadRight);
                returnValue = loadRight;
            } else {
                returnValue = right->val;
            }

            // 类型转换检查
            if (returnType->isFloatType() && !returnValue->getType()->isFloatType()) {
                // 需要转换为浮点数
                returnValue = convertToFloat(returnValue, currentFunc, node->blockInsts);
            } else if (returnType->isIntegerType() && !returnValue->getType()->isIntegerType()) {
                // 需要转换为整数
                returnValue = convertToInt(returnValue, currentFunc, node->blockInsts);
            }
        }
    } else if (!returnType->isVoidType()) {
        printf("Error: Non-void function should return a value.\n");
        return false;
    }

    // 处理函数返回值变量（如果有）
    LocalVariable * retVar = currentFunc->getReturnValue();
    if (retVar && returnValue) {
        // 存储返回值到返回值变量
        StoreInstruction * storeRet = new StoreInstruction(currentFunc, returnValue, retVar, returnType->getSize());
        node->blockInsts.addInst(storeRet);
    }

    // 跳转到函数出口
    Instruction * exitLabel = currentFunc->getExitLabel();
    if (!exitLabel) {
        printf("Error: No exit label defined for function.\n");
        return false;
    }

    GotoInstruction * gotoExit = new GotoInstruction(currentFunc, exitLabel);
    node->blockInsts.addInst(gotoExit);

    // 设置节点值为返回值（可能为nullptr）
    node->val = returnValue;

    return true;
}

/// @brief 类型叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_type(ast_node * node)
{
    // 不需要做什么，直接从节点中获取即可。

    return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
    Value * val;

    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值

    val = module->findVarValue(node->name);

    node->val = val;

    return true;
}

/// @brief 无符号整数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_uint(ast_node * node)
{
    std::string numStr = node->name;
    int32_t value = 0;

    try {
        // 判断数字类型并解析
        if (numStr.size() >= 2 && (numStr.substr(0, 2) == "0x" || numStr.substr(0, 2) == "0X")) {
            // 十六进制数字 - 使用stoull避免溢出，然后转换为int32_t
            uint64_t temp = std::stoull(numStr, nullptr, 16);
            // 将无符号32位值重新解释为有符号32位值
            value = static_cast<int32_t>(static_cast<uint32_t>(temp));
        } else if (numStr.size() >= 2 && numStr[0] == '0' && numStr[1] >= '0' && numStr[1] <= '7') {
            // 八进制数字（以0开头且第二个字符是八进制数字）
            uint64_t temp = std::stoull(numStr, nullptr, 8);
            value = static_cast<int32_t>(static_cast<uint32_t>(temp));
        } else {
            // 十进制数字
            uint64_t temp = std::stoull(numStr, nullptr, 10);
            value = static_cast<int32_t>(static_cast<uint32_t>(temp));
        }
    } catch (const std::exception & e) {
        printf("Error: Failed to parse integer literal '%s': %s\n", numStr.c_str(), e.what());
        return false;
    }

    // 新建常量
    ConstInt * newConst = module->newConstInt(value);

    // 设置节点的值
    node->val = newConst;

    return true;
}

/// @brief 浮点数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_float(ast_node * node)
{
    float value = node->float_val;

    // 新建浮点数常量
    ConstFloat * newConst = module->newConstFloat(value);

    // 设置节点的值
    node->val = newConst;

    return true;
}

/// @brief 变量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_declare_statment(ast_node * node)
{
    bool result = false;

    for (auto & child: node->sons) {

        // 遍历每个变量声明
        result = ir_variable_declare(child);
        if (!result) {
            break;
        }
        // 收集子节点生成的 IR
        node->blockInsts.addInst(child->blockInsts);
    }

    return result;
}

/// @brief 变量定声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_variable_declare(ast_node * node)
{
    // 确保节点有两个子节点：类型节点和变量名或赋值节点
    if (node->sons.size() < 2) {
        printf("Error: Invalid node structure in ir_variable_declare. Expected 2 children, got %zu.\n",
               node->sons.size());
        return false;
    }

    ast_node * typeNode = node->sons[0];
    ast_node * varOrAssignNode = node->sons[1];

    if (!typeNode || !varOrAssignNode) {
        printf("Error: Null typeNode or varOrAssignNode in ir_variable_declare.\n");
        return false;
    }

    // 处理变量声明（带或不带初始化）
    ast_node * varNode = nullptr;
    ast_node * initExprNode = nullptr;

    if (varOrAssignNode->node_type == ast_operator_type::AST_OP_ASSIGN) {
        // 带初始化的声明
        if (varOrAssignNode->sons.size() < 2) {
            printf("Error: Invalid assignment structure in ir_variable_declare.\n");
            return false;
        }

        varNode = varOrAssignNode->sons[0];
        initExprNode = varOrAssignNode->sons[1];

        if (!varNode || !initExprNode) {
            printf("Error: Null varNode or initExprNode in assignment.\n");
            return false;
        }
    } else {
        // 不带初始化的声明
        varNode = varOrAssignNode;
    }

    // 检查变量节点是否有维度信息（数组声明）
    // 但是如果typeNode->type已经是ArrayType，说明前端已经处理过了，不需要重复处理
    if (!varNode->sons.empty() && !typeNode->type->isArrayType()) {
        // 变量节点有子节点，说明是数组声明，需要创建数组类型
        std::vector<int> dimensions;
        for (auto dimNode: varNode->sons) {
            if (dimNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                dimensions.push_back(dimNode->integer_val);
            } else {
                printf("Error: Non-constant array dimension in variable declaration.\n");
                return false;
            }
        }

        // printf("Debug: Creating ArrayType with elementType=%s, dimensions=[", typeNode->type->toString().c_str());
        for (size_t i = 0; i < dimensions.size(); ++i) {
            if (i > 0)
                printf(", ");
            printf("%d", dimensions[i]);
        }
        printf("]\n");

        // 创建数组类型
        ArrayType * arrayType = new ArrayType(typeNode->type, dimensions);

        // printf("Debug: Created ArrayType: %s\n", arrayType->toString().c_str());

        // 更新类型节点
        typeNode->type = arrayType;
    } else if (typeNode->type->isArrayType()) {
        // printf("Debug: Type is already ArrayType: %s, skipping dimension processing\n",
        // typeNode->type->toString().c_str());
    }

    // 检查当前是否在全局作用域
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        return ir_global_variable_declare(node, typeNode, varOrAssignNode);
    }

    // 检查是否是数组类型
    if (typeNode->type->isArrayType() ||
        (initExprNode && initExprNode->node_type == ast_operator_type::AST_OP_ARRAY_INIT)) {
        return ir_array_variable_declare_with_init(node, typeNode, varNode, initExprNode);
    }

    // 检查变量是否已经在函数开始时预分配
    Value * varValue = varNode->val; // 应该在generateAllocaInstructions中已经设置
    if (varValue) {
        // 变量已经预分配，现在需要将其注册到当前作用域
        // printf("Debug: Using pre-allocated variable '%s', registering to current scope\n", varNode->name.c_str());

        // 将预分配的变量注册到当前作用域
        // 这会覆盖同名的外层变量，实现正确的作用域遮蔽
        if (!registerVariableToCurrentScope(varNode->name, varValue)) {
            printf("Error: Failed to register pre-allocated variable '%s' to current scope\n", varNode->name.c_str());
            return false;
        }
    } else {
        // 变量未预分配，按原来的方式创建（用于全局变量等）
        Type * allocaType = typeNode->type;           // 要分配的类型
        Type * varType = new PointerType(allocaType); // 变量类型（指针类型）
        varValue = module->newVarValue(varType, varNode->name);
        if (!varValue) {
            printf("Error: Failed to allocate variable '%s' in ir_variable_declare.\n", varNode->name.c_str());
            return false;
        }

        // 计算对齐大小（基本类型使用类型大小，数组使用16字节对齐）
        uint32_t alignSize = allocaType->isArrayType() ? 16 : allocaType->getSize();

        AllocaInstruction * allocaInst = new AllocaInstruction(currentFunc, varValue, allocaType, alignSize);
        node->blockInsts.addInst(allocaInst);
        varNode->val = varValue;

        // printf("Debug: Created new variable '%s' with alloca\n", varNode->name.c_str());
    }

    // 处理初始化部分
    if (initExprNode) {
        // 处理初始化表达式
        if (!ir_visit_ast_node(initExprNode)) {
            printf("Error: Failed to evaluate initialization expression for '%s'.\n", varNode->name.c_str());
            return false;
        }

        Value * initValue = nullptr;

        // 处理浮点数常量初始化
        if (initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
            initValue = module->newConstFloat(initExprNode->float_val);
            node->blockInsts.addInst(initExprNode->blockInsts);
        }
        // 处理数组访问的情况
        else if (initExprNode->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
            // 数组访问返回地址，需要加载值
            node->blockInsts.addInst(initExprNode->blockInsts);

            // 获取数组元素类型来确定加载大小
            const Type * elementType = nullptr;
            if (initExprNode->val->getType()->isPointerType()) {
                const PointerType * ptrType = static_cast<const PointerType *>(initExprNode->val->getType());
                elementType = ptrType->getPointeeType();
            } else {
                printf("Error: Array access should return pointer type.\n");
                return false;
            }

            // 对于基础类型，使用固定的对齐方式
            int alignment = 4; // 默认对齐方式
            if (elementType->isFloatType()) {
                alignment = 4; // float 类型使用 4 字节对齐
            } else if (elementType->isIntegerType()) {
                alignment = 4; // int 类型使用 4 字节对齐
            }

            LoadInstruction * loadInit =
                new LoadInstruction(currentFunc, initExprNode->val, initExprNode->val, alignment);
            node->blockInsts.addInst(loadInit);
            initValue = loadInit;
        }
        // 处理需要加载的情况
        else if (needsLoad(initExprNode->val)) {
            // 对于需要加载的值，使用固定的对齐方式
            int alignment = 4; // 默认对齐方式

            // 如果是指针类型，获取指向的类型来确定对齐方式
            if (initExprNode->val->getType()->isPointerType()) {
                const PointerType * ptrType = static_cast<const PointerType *>(initExprNode->val->getType());
                const Type * pointeeType = ptrType->getPointeeType();
                if (pointeeType->isFloatType()) {
                    alignment = 4; // float 类型使用 4 字节对齐
                } else if (pointeeType->isIntegerType()) {
                    alignment = 4; // int 类型使用 4 字节对齐
                }
            }

            LoadInstruction * loadInit =
                new LoadInstruction(currentFunc, initExprNode->val, initExprNode->val, alignment);
            node->blockInsts.addInst(initExprNode->blockInsts);
            node->blockInsts.addInst(loadInit);
            initValue = loadInit;
        } else {
            // 直接使用常量或表达式结果
            node->blockInsts.addInst(initExprNode->blockInsts);
            initValue = initExprNode->val;
        }

        // 类型转换检查
        if (initValue) {
            if (typeNode->type->isFloatType() && !initValue->getType()->isFloatType()) {
                // 目标类型是浮点数，源类型不是，需要转换
                initValue = convertToFloat(initValue, currentFunc, node->blockInsts);
                if (!initValue) {
                    printf("Error: Failed to convert to float for variable '%s'.\n", varNode->name.c_str());
                    return false;
                }
            } else if (typeNode->type->isIntegerType() && !initValue->getType()->isIntegerType()) {
                // 目标类型是整数，源类型不是，需要转换
                initValue = convertToInt(initValue, currentFunc, node->blockInsts);
                if (!initValue) {
                    printf("Error: Failed to convert to int for variable '%s'.\n", varNode->name.c_str());
                    return false;
                }
            }

            // 存储初始化值
            StoreInstruction * storeInst =
                new StoreInstruction(currentFunc, initValue, varValue, typeNode->type->getSize());
            node->blockInsts.addInst(storeInst);
        }
    }

    return true;
}

/// @brief 全局变量声明节点翻译成线性中间IR
/// @param node AST节点
/// @param typeNode 类型节点
bool IRGenerator::ir_global_variable_declare(ast_node * node, ast_node * typeNode, ast_node * varOrAssignNode)
{
    // 如果是赋值节点（AST_OP_ASSIGN）
    if (varOrAssignNode->node_type == ast_operator_type::AST_OP_ASSIGN) {
        // 获取赋值节点的子节点：变量名和初值表达式
        ast_node * varNode = varOrAssignNode->sons[0];
        ast_node * initExprNode = varOrAssignNode->sons[1];

        if (!varNode || !initExprNode) {
            printf("Error: Invalid global variable assignment structure.\n");
            return false;
        }

        // 检查是否是数组初始化
        if (initExprNode->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
            // 对于全局数组变量的初始化，直接在这里处理，不调用ir_array_variable_declare_with_init
            if (typeNode->type->isArrayType()) {
                ArrayType * arrayType = static_cast<ArrayType *>(typeNode->type);

                // 创建全局数组变量（非常量）
                Value * globalVar = module->newVarValue(arrayType, varNode->name);
                if (!globalVar) {
                    printf("Error: Failed to create global array variable '%s'.\n", varNode->name.c_str());
                    return false;
                }

                // 转换为 GlobalVariable 并设置属性
                GlobalVariable * globalArray = static_cast<GlobalVariable *>(globalVar);
                globalArray->setConstant(false);   // 非常量
                globalArray->setBSSSection(false); // 有初值，不在BSS段
                globalArray->setAlignment(4);      // 设置4字节对齐

                // 处理初始化值
                std::vector<Value *> initValues;
                if (!processArrayInitialization(initExprNode, arrayType, initValues)) {
                    printf("Error: Failed to process array initialization for global variable '%s'.\n",
                           varNode->name.c_str());
                    return false;
                }

                // 设置初始化值列表
                globalArray->setInitValueList(initValues);

                // 设置节点的Value
                varNode->val = globalVar;
                node->val = globalVar;

                // printf("Debug: Created global array variable '%s' with %zu initialization values\n",
                // varNode->name.c_str(),
                // initValues.size());

                return true;
            } else {
                printf("Error: Array initialization for non-array type.\n");
                return false;
            }
        }

        // 处理标量初值
        Value * initValue = nullptr;
        if (initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
            if (typeNode->type->isFloatType()) {
                initValue = module->newConstFloat(static_cast<float>(initExprNode->integer_val));
            } else {
                initValue = module->newConstInt(initExprNode->integer_val);
            }
        } else if (initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
            if (typeNode->type->isIntegerType()) {
                initValue = module->newConstInt(static_cast<int32_t>(initExprNode->float_val));
            } else {
                initValue = module->newConstFloat(initExprNode->float_val);
            }
        } else if (initExprNode->node_type == ast_operator_type::AST_OP_NEGATIVE) {
            // 处理负数，如 int a = -1;
            if (initExprNode->sons.size() == 1) {
                ast_node * operandNode = initExprNode->sons[0];
                if (operandNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                    int32_t negativeValue = -static_cast<int32_t>(operandNode->integer_val);
                    if (typeNode->type->isFloatType()) {
                        initValue = module->newConstFloat(static_cast<float>(negativeValue));
                    } else {
                        initValue = module->newConstInt(negativeValue);
                    }
                } else if (operandNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
                    float negativeValue = -operandNode->float_val;
                    if (typeNode->type->isIntegerType()) {
                        initValue = module->newConstInt(static_cast<int32_t>(negativeValue));
                    } else {
                        initValue = module->newConstFloat(negativeValue);
                    }
                } else {
                    printf("Error: Global variable initialization with negative operator only supports literal "
                           "operands.\n");
                    return false;
                }
            } else {
                printf("Error: Invalid negative operator in global variable initialization.\n");
                return false;
            }
        } else {
            printf("Error: Global variable initialization only supports constants and arrays. Got node type: %d\n",
                   static_cast<int>(initExprNode->node_type));
            return false;
        }

        // printf("Debug: Creating global variable '%s' with type: %s\n",
        // varNode->name.c_str(),
        // typeNode->type->toString().c_str());

        // 使用公有的 newVarValue 方法创建全局变量
        // 由于currentFunc为nullptr，会自动调用newGlobalVariable
        Value * globalVar = module->newVarValue(typeNode->type, varNode->name);
        if (!globalVar) {
            printf("Error: Failed to create global variable.\n");
            return false;
        }

        // printf("Debug: Successfully created global variable '%s'\n", varNode->name.c_str());

        // 将全局变量转换为 GlobalVariable 类型并设置初值
        GlobalVariable * globalVariable = static_cast<GlobalVariable *>(globalVar);
        // TODO: 需要在 GlobalVariable 类中添加 setInitValue 方法
        globalVariable->setInitValue(initValue);
        if (initExprNode->integer_val != 0) {
            globalVariable->setBSSSection(false);
        }
        // 设置节点的Value
        varNode->val = globalVar;
        node->val = globalVar;

        return true;

    } else {
        // 如果是普通全局变量声明（没有初值）
        ast_node * varNode = varOrAssignNode;

        // printf("Debug: Creating global variable '%s' with type: %s\n",
        // varNode->name.c_str(),
        // typeNode->type->toString().c_str());

        // 使用公有的 newVarValue 方法创建全局变量
        Value * globalVar = module->newVarValue(typeNode->type, varNode->name);
        if (!globalVar) {
            printf("Error: Failed to create global variable.\n");
            return false;
        }

        // printf("Debug: Successfully created global variable '%s'\n", varNode->name.c_str());

        // 设置节点的Value
        varNode->val = globalVar;
        node->val = globalVar;

        return true;
    }
}

/// @brief 关系表达式AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_rel_exp(ast_node * node)
{
    // 确保节点有两个子节点：左操作数和右操作数
    if (node->sons.size() != 2) {
        printf("Error: Invalid node structure in ir_rel_exp.\n");
        return false;
    }

    // 获取左操作数和右操作数
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左操作数和右操作数的 IR
    if (!ir_visit_ast_node(leftNode) || !ir_visit_ast_node(rightNode)) {
        return false;
    }

    // 获取操作符类型
    IRInstOperator op;
    switch (node->node_type) {
        case ast_operator_type::AST_OP_LT:
            op = IRInstOperator::IRINST_OP_LT;
            break;
        case ast_operator_type::AST_OP_GT:
            op = IRInstOperator::IRINST_OP_GT;
            break;
        case ast_operator_type::AST_OP_LE:
            op = IRInstOperator::IRINST_OP_LE;
            break;
        case ast_operator_type::AST_OP_GE:
            op = IRInstOperator::IRINST_OP_GE;
            break;
        case ast_operator_type::AST_OP_EQ:
            op = IRInstOperator::IRINST_OP_EQ;
            break;
        case ast_operator_type::AST_OP_NE:
            op = IRInstOperator::IRINST_OP_NE;
            break;
        default:
            printf("Error: Unsupported operator in ir_rel_exp.\n");
            return false;
    }

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 处理左操作数
    if (needsLoad(leftNode->val)) {
        // 左操作数是变量，需要加载
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), leftNode->val, leftNode->val, 4);
        node->blockInsts.addInst(leftNode->blockInsts);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        // 左操作数是常量或表达式结果，直接使用
        node->blockInsts.addInst(leftNode->blockInsts);
        leftValue = leftNode->val;
    }

    // 修复：处理右操作数
    if (needsLoad(rightNode->val)) {
        // 右操作数是变量，需要加载
        LoadInstruction * loadRight =
            new LoadInstruction(module->getCurrentFunction(), rightNode->val, rightNode->val, 4);
        node->blockInsts.addInst(rightNode->blockInsts);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        // 右操作数是常量或表达式结果，直接使用
        node->blockInsts.addInst(rightNode->blockInsts);
        rightValue = rightNode->val;
    }

    // 类型转换：如果任一操作数是浮点数，将另一个操作数也转换为浮点数
    Type * leftType = leftValue->getType();
    Type * rightType = rightValue->getType();

    if (leftType->isFloatType() || rightType->isFloatType()) {
        // 确保两个操作数都是浮点数类型
        if (!leftValue->getType()->isFloatType()) {
            // 特殊处理常量：如果是整数常量，直接创建对应的浮点常量
            if (ConstInt * constInt = dynamic_cast<ConstInt *>(leftValue)) {
                leftValue = module->newConstFloat(static_cast<float>(constInt->getVal()));
            } else {
                leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
                if (!leftValue) {
                    printf("Error: Failed to convert left operand to float type in comparison.\n");
                    return false;
                }
            }
        }
        if (!rightValue->getType()->isFloatType()) {
            // 特殊处理常量：如果是整数常量，直接创建对应的浮点常量
            if (ConstInt * constInt = dynamic_cast<ConstInt *>(rightValue)) {
                rightValue = module->newConstFloat(static_cast<float>(constInt->getVal()));
            } else {
                rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
                if (!rightValue) {
                    printf("Error: Failed to convert right operand to float type in comparison.\n");
                    return false;
                }
            }
        }
    } else {
        // 处理整数和布尔值的比较
        // 如果一个操作数是i1类型，另一个是i32类型，需要进行类型转换
        if (leftType->isInt1Byte() && rightType->isInt32Type()) {
            // 将i1类型转换为i32类型
            leftValue = convertToI32(leftValue, module->getCurrentFunction(), node->blockInsts);
        } else if (leftType->isInt32Type() && rightType->isInt1Byte()) {
            // 将i1类型转换为i32类型
            rightValue = convertToI32(rightValue, module->getCurrentFunction(), node->blockInsts);
        }
    }

    // 创建关系表达式指令
    auto * relInst =
        new RelInstruction(module->getCurrentFunction(), op, leftValue, rightValue, IntegerType::getTypeBool());

    // 将指令添加到当前节点的指令块
    node->val = relInst;
    node->blockInsts.addInst(relInst);

    return true;
}

bool IRGenerator::ir_while(ast_node * node)
{
    // 确保节点有两个子节点：条件表达式和循环体
    if (node->sons.size() != 2) {
        printf("Error: Invalid node structure in ir_while.\n");
        return false;
    }

    ast_node * condNode = node->sons[0]; // 条件表达式
    ast_node * bodyNode = node->sons[1]; // 循环体

    // 获取当前函数
    Function * currentFunc = module->getCurrentFunction();

    // 创建循环的条件、循环体和退出标签
    LabelInstruction * condLabel = new LabelInstruction(currentFunc, module);
    LabelInstruction * bodyLabel = new LabelInstruction(currentFunc, module);
    LabelInstruction * exitLabel = new LabelInstruction(currentFunc, module);

    // 条件检查标签和推出标签压栈
    loopLabelStack.push({condLabel, exitLabel});

    // 直接跳转到条件检查（不需要额外的入口标签）
    node->blockInsts.addInst(new GotoInstruction(currentFunc, condLabel));

    // 条件检查标签
    node->blockInsts.addInst(condLabel);

    // 生成条件表达式的 IR，使用新的条件表达式处理方法
    if (!ir_condition_expr(condNode, bodyLabel, exitLabel)) {
        return false;
    }
    node->blockInsts.addInst(condNode->blockInsts);

    // 条件跳转指令：条件为真跳转到循环体，为假跳转到退出标签
    // node->blockInsts.addInst(new BranchInstruction(currentFunc, condNode->val, bodyLabel, exitLabel));

    // 循环体标签
    node->blockInsts.addInst(bodyLabel);

    // 生成循环体的 IR
    if (!ir_visit_ast_node(bodyNode)) {
        return false;
    }
    node->blockInsts.addInst(bodyNode->blockInsts);

    // 跳转回条件检查
    node->blockInsts.addInst(new GotoInstruction(currentFunc, condLabel));

    // 退出标签
    node->blockInsts.addInst(exitLabel);

    // 弹出循环标签栈
    loopLabelStack.pop();

    return true;
}

bool IRGenerator::ir_if_else(ast_node * node)
{
    // 确保节点有两个或三个子节点：条件表达式、then分支、（可选的）else分支
    if (node->sons.size() < 2 || node->sons.size() > 3) {
        printf("Error: Invalid node structure in ir_if_else.\n");
        return false;
    }

    ast_node * condNode = node->sons[0];                                      // 条件表达式
    ast_node * thenNode = node->sons[1];                                      // then分支
    ast_node * elseNode = (node->sons.size() == 3) ? node->sons[2] : nullptr; // else分支（可选）

    // 获取当前函数
    Function * currentFunc = module->getCurrentFunction();

    // 创建then分支、else分支（可选）和结束标签
    LabelInstruction * thenLabel = new LabelInstruction(currentFunc, module);
    LabelInstruction * elseLabel = elseNode ? new LabelInstruction(currentFunc, module) : nullptr;
    LabelInstruction * endLabel = new LabelInstruction(currentFunc, module);

    // 生成条件表达式的 IR，使用新的条件表达式处理方法
    if (!ir_condition_expr(condNode, thenLabel, elseLabel ? elseLabel : endLabel)) {
        return false;
    }
    node->blockInsts.addInst(condNode->blockInsts);

    // then分支标签
    node->blockInsts.addInst(thenLabel);

    // 生成then分支的 IR
    if (!ir_visit_ast_node(thenNode)) {
        return false;
    }
    // 检查then分支本身是否已经有终结指令（如break、continue、return）
    // 必须在addInst之前检查，因为addInst会清空原来的指令序列
    bool thenHasTerminator = hasTerminatorInstruction(thenNode->blockInsts);

    node->blockInsts.addInst(thenNode->blockInsts);
    if (!thenHasTerminator) {
        // 只有在没有终结指令时才添加跳转到结束标签
        node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));
    }

    // else分支标签（如果存在）
    if (elseNode) {
        node->blockInsts.addInst(elseLabel);

        // 生成else分支的 IR
        if (!ir_visit_ast_node(elseNode)) {
            return false;
        }

        // 检查else分支本身是否已经有终结指令
        // 必须在addInst之前检查，因为addInst会清空原来的指令序列
        bool elseHasTerminator = hasTerminatorInstruction(elseNode->blockInsts);

        node->blockInsts.addInst(elseNode->blockInsts);
        if (!elseHasTerminator) {
            // 只有在没有终结指令时才添加跳转到结束标签
            node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));
        }
    }

    // 结束标签
    node->blockInsts.addInst(endLabel);

    return true;
}

bool IRGenerator::ir_break(ast_node * node)
{
    // printf("=== IR_BREAK: Processing break statement ===\n");

    if (loopLabelStack.empty()) {
        printf("Error: break statement not inside a loop.\n");
        return false;
    }

    // 获取当前循环的退出标签
    LabelInstruction * exitLabel = loopLabelStack.top().exitLabel;
    // printf("=== IR_BREAK: Generating goto to exit label %s ===\n", exitLabel->getIRName().c_str());

    // 生成跳转到退出标签的指令
    node->blockInsts.addInst(new GotoInstruction(module->getCurrentFunction(), exitLabel));

    return true;
}

bool IRGenerator::ir_continue(ast_node * node)
{
    // printf("=== IR_CONTINUE: Processing continue statement ===\n");

    if (loopLabelStack.empty()) {
        printf("Error: continue statement not inside a loop.\n");
        return false;
    }

    // 获取当前循环的条件检查标签
    LabelInstruction * condLabel = loopLabelStack.top().condLabel;
    // printf("=== IR_CONTINUE: Generating goto to condition label %s ===\n", condLabel->getIRName().c_str());

    // 生成跳转到条件检查标签的指令
    node->blockInsts.addInst(new GotoInstruction(module->getCurrentFunction(), condLabel));

    return true;
}

/// @brief 整数取模AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mod(ast_node * node)
{
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左右孩子的IR
    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right)
        return false;

    // 常量折叠检查：如果两个操作数都是常量，直接计算结果
    ConstInt * leftConstInt = dynamic_cast<ConstInt *>(left->val);
    ConstInt * rightConstInt = dynamic_cast<ConstInt *>(right->val);

    if (leftConstInt && rightConstInt) {
        // 常量折叠：两个操作数都是整数常量
        // printf("Debug: Performing constant folding for modulo\n");

        // 检查除零错误
        if (rightConstInt->getVal() == 0) {
            printf("Error: Modulo by zero in constant folding\n");
            return false;
        }

        int32_t leftVal = leftConstInt->getVal();
        int32_t rightVal = rightConstInt->getVal();
        int32_t result = leftVal % rightVal;

        ConstInt * resultConst = module->newConstInt(result);
        node->val = resultConst;
        // printf("Debug: Constant folding result: %d %% %d = %d\n", leftVal, rightVal, result);
        return true;
    }

    // 合并左右孩子的IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 生成取模指令
    BinaryInstruction * modInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MOD_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(modInst);
    node->val = modInst;

    return true;
}

/// @brief 正号一元运算符AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_positive(ast_node * node)
{
    // 正号运算符实际上不改变值，直接返回操作数的值
    ast_node * operandNode = node->sons[0];

    ast_node * operand = ir_visit_ast_node(operandNode);
    if (!operand) {
        return false;
    }

    // 复制操作数的指令和值
    node->blockInsts.addInst(operand->blockInsts);
    node->val = operand->val;

    return true;
}

/// @brief 负号一元运算符AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_negative(ast_node * node)
{
    ast_node * operandNode = node->sons[0];

    ast_node * operand = ir_visit_ast_node(operandNode);
    if (!operand) {
        return false;
    }

    node->blockInsts.addInst(operand->blockInsts);

    Value * operandValue = nullptr;

    // 处理操作数
    if (needsLoad(operand->val)) {
        LoadInstruction * loadOperand =
            new LoadInstruction(module->getCurrentFunction(), operand->val, operand->val, 4);
        node->blockInsts.addInst(loadOperand);
        operandValue = loadOperand;
    } else {
        operandValue = operand->val;
    }

    // 检查操作数类型
    Type * operandType = operandValue->getType();

    if (operandType->isInt1Byte()) {
        // 对于i1类型（布尔类型），需要先转换为int32类型，然后求负
        // 创建一个int32类型的局部变量
        LocalVariable * tempVar =
            static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt(), "__temp_bool_to_int"));
        AllocaInstruction * allocaInst =
            new AllocaInstruction(module->getCurrentFunction(), tempVar, IntegerType::getTypeInt(), 4);
        node->blockInsts.addInst(allocaInst);

        // 创建标签
        LabelInstruction * trueLabel = new LabelInstruction(module->getCurrentFunction(), module);
        LabelInstruction * falseLabel = new LabelInstruction(module->getCurrentFunction(), module);
        LabelInstruction * endLabel = new LabelInstruction(module->getCurrentFunction(), module);

        // 根据i1值进行分支
        BranchInstruction * branchInst =
            new BranchInstruction(module->getCurrentFunction(), operandValue, trueLabel, falseLabel);
        node->blockInsts.addInst(branchInst);

        // true分支：存储1
        node->blockInsts.addInst(trueLabel);
        ConstInt * oneConst = module->newConstInt(1);
        StoreInstruction * storeOne = new StoreInstruction(module->getCurrentFunction(), oneConst, tempVar, 4);
        node->blockInsts.addInst(storeOne);
        GotoInstruction * gotoEnd1 = new GotoInstruction(module->getCurrentFunction(), endLabel);
        node->blockInsts.addInst(gotoEnd1);

        // false分支：存储0
        node->blockInsts.addInst(falseLabel);
        ConstInt * zeroConst = module->newConstInt(0);
        StoreInstruction * storeZero = new StoreInstruction(module->getCurrentFunction(), zeroConst, tempVar, 4);
        node->blockInsts.addInst(storeZero);
        GotoInstruction * gotoEnd2 = new GotoInstruction(module->getCurrentFunction(), endLabel);
        node->blockInsts.addInst(gotoEnd2);

        // 结束标签
        node->blockInsts.addInst(endLabel);

        // 加载转换后的int32值
        LoadInstruction * loadInt = new LoadInstruction(module->getCurrentFunction(), tempVar, tempVar, 4);
        node->blockInsts.addInst(loadInt);

        // 生成负号指令：0 - loadInt
        ConstInt * zeroForNeg = module->newConstInt(0);
        BinaryInstruction * negInst = new BinaryInstruction(module->getCurrentFunction(),
                                                            IRInstOperator::IRINST_OP_SUB_I,
                                                            zeroForNeg,
                                                            loadInt,
                                                            IntegerType::getTypeInt());
        node->blockInsts.addInst(negInst);
        node->val = negInst;
    } else if (operandType->isFloatType()) {
        // 对于float类型，生成浮点数负号指令：0.0 - operand
        ConstFloat * zeroFloatConst = module->newConstFloat(0.0f);
        BinaryInstruction * negInst = new BinaryInstruction(module->getCurrentFunction(),
                                                            IRInstOperator::IRINST_OP_SUB_F,
                                                            zeroFloatConst,
                                                            operandValue,
                                                            FloatType::getTypeFloat());
        node->blockInsts.addInst(negInst);
        node->val = negInst;
    } else {
        // 对于int32类型，直接生成负号指令：0 - operand
        ConstInt * zeroConst = module->newConstInt(0);
        BinaryInstruction * negInst = new BinaryInstruction(module->getCurrentFunction(),
                                                            IRInstOperator::IRINST_OP_SUB_I,
                                                            zeroConst,
                                                            operandValue,
                                                            IntegerType::getTypeInt());
        node->blockInsts.addInst(negInst);
        node->val = negInst;
    }

    return true;
}

/// @brief 处理算术运算中的类型转换（i1到i32的零扩展）
/// @param node AST节点
/// @param leftValue 左操作数值（可能被修改）
/// @param rightValue 右操作数值（可能被修改）
/// @return 是否成功处理类型转换
bool IRGenerator::handleArithmeticTypeConversion(ast_node * node, Value *& leftValue, Value *& rightValue)
{
    Type * leftType = leftValue->getType();
    Type * rightType = rightValue->getType();

    // 处理i1类型与i32类型的混合运算
    if (leftType->isInt1Byte() && rightType->isInt32Type()) {
        // 将左操作数从i1零扩展为i32
        ZextInstruction * zextLeft =
            new ZextInstruction(module->getCurrentFunction(), leftValue, IntegerType::getTypeInt());
        node->blockInsts.addInst(zextLeft);
        leftValue = zextLeft;
    } else if (leftType->isInt32Type() && rightType->isInt1Byte()) {
        // 将右操作数从i1零扩展为i32
        ZextInstruction * zextRight =
            new ZextInstruction(module->getCurrentFunction(), rightValue, IntegerType::getTypeInt());
        node->blockInsts.addInst(zextRight);
        rightValue = zextRight;
    } else if (leftType->isInt1Byte() && rightType->isInt1Byte()) {
        // 两个操作数都是i1类型，都扩展为i32
        ZextInstruction * zextLeft =
            new ZextInstruction(module->getCurrentFunction(), leftValue, IntegerType::getTypeInt());
        node->blockInsts.addInst(zextLeft);
        leftValue = zextLeft;

        ZextInstruction * zextRight =
            new ZextInstruction(module->getCurrentFunction(), rightValue, IntegerType::getTypeInt());
        node->blockInsts.addInst(zextRight);
        rightValue = zextRight;
    }

    return true;
}

/// @brief 翻转比较操作符
/// @param op 原始比较操作符
/// @return 翻转后的比较操作符
IRInstOperator flipComparisonOperator(IRInstOperator op)
{
    switch (op) {
        case IRInstOperator::IRINST_OP_EQ:
            return IRInstOperator::IRINST_OP_NE;
        case IRInstOperator::IRINST_OP_NE:
            return IRInstOperator::IRINST_OP_EQ;
        case IRInstOperator::IRINST_OP_LT:
            return IRInstOperator::IRINST_OP_GE;
        case IRInstOperator::IRINST_OP_LE:
            return IRInstOperator::IRINST_OP_GT;
        case IRInstOperator::IRINST_OP_GT:
            return IRInstOperator::IRINST_OP_LE;
        case IRInstOperator::IRINST_OP_GE:
            return IRInstOperator::IRINST_OP_LT;
        default:
            // 对于其他操作符，返回原操作符（不应该发生）
            return op;
    }
}

/// @brief 逻辑非运算符AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_not(ast_node * node)
{
    ast_node * operandNode = node->sons[0];

    // 递归处理操作数
    ast_node * operand = ir_visit_ast_node(operandNode);
    if (!operand) {
        return false;
    }

    // 合并操作数的指令
    node->blockInsts.addInst(operand->blockInsts);

    Value * operandValue = nullptr;

    // 检查操作数是否需要加载
    if (needsLoad(operand->val)) {
        LoadInstruction * loadInst = new LoadInstruction(module->getCurrentFunction(), operand->val, operand->val, 4);
        node->blockInsts.addInst(loadInst);
        operandValue = loadInst;
    } else {
        operandValue = operand->val;
    }

    // 根据操作数的类型决定如何处理逻辑非
    Type * operandType = operandValue->getType();

    if (operandType->isInt32Type()) {
        // 对于int32类型，创建比较指令：result = (operand == 0)
        ConstInt * zeroConst = module->newConstInt(0);
        RelInstruction * cmpInst = new RelInstruction(module->getCurrentFunction(),
                                                      IRInstOperator::IRINST_OP_EQ,
                                                      operandValue,
                                                      zeroConst,
                                                      IntegerType::getTypeBool());
        node->blockInsts.addInst(cmpInst);
        node->val = cmpInst;
    } else if (operandType->isInt1Byte()) {
        // 对于i1类型（布尔类型），翻转比较结果
        // 检查操作数是否是比较指令，如果是则翻转比较操作符
        if (RelInstruction * relInst = dynamic_cast<RelInstruction *>(operandValue)) {
            // 翻转比较操作符
            IRInstOperator flippedOp = flipComparisonOperator(relInst->getOp());
            RelInstruction * flippedInst = new RelInstruction(module->getCurrentFunction(),
                                                              flippedOp,
                                                              relInst->getOperand(0),
                                                              relInst->getOperand(1),
                                                              IntegerType::getTypeBool());
            node->blockInsts.addInst(flippedInst);
            node->val = flippedInst;
        } else {
            // 对于其他i1类型值，使用XOR与1进行翻转
            ConstInt * oneConst = module->newConstInt(1);
            XorInstruction * xorInst =
                new XorInstruction(module->getCurrentFunction(), operandValue, oneConst, IntegerType::getTypeBool());
            node->blockInsts.addInst(xorInst);
            node->val = xorInst;
        }
    } else {
        printf("Error: Logical NOT operator can only be applied to integer or boolean types.\n");
        return false;
    }

    return true;
}

/// @brief 逻辑与运算符AST节点翻译成线性中间IR（兼容旧接口）
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_and(ast_node * node)
{
    // 这个方法不应该被直接调用，因为逻辑表达式需要外部提供真假出口
    // 如果被调用，说明逻辑表达式被错误地用作普通表达式
    printf("Error: Logical AND expression cannot be used as regular expression.\n");
    return false;
}

/// @brief 逻辑或运算符AST节点翻译成线性中间IR（兼容旧接口）
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_or(ast_node * node)
{
    // 这个方法不应该被直接调用，因为逻辑表达式需要外部提供真假出口
    // 如果被调用，说明逻辑表达式被错误地用作普通表达式
    printf("Error: Logical OR expression cannot be used as regular expression.\n");
    return false;
}

bool IRGenerator::needsLoad(Value * val)
{
    // 如果值为空，不需要加载
    if (!val)
        return false;

    // 如果是常量，不需要加载
    if (dynamic_cast<ConstInt *>(val) != nullptr) {
        return false;
    }

    // 如果是浮点数常量，不需要加载
    if (dynamic_cast<ConstFloat *>(val) != nullptr) {
        return false;
    }

    // 如果是形参，不需要加载
    if (dynamic_cast<FormalParam *>(val) != nullptr) {
        return false;
    }

    // 检查是否是指令结果
    if (Instruction * inst = dynamic_cast<Instruction *>(val)) {
        // GetelementptrInstruction 的结果是地址，需要加载来获取值
        if (dynamic_cast<GetelementptrInstruction *>(inst) != nullptr) {
            return true;
        }
        // 其他指令结果（如算术运算、load指令等）不需要再次加载
        return false;
    }

    // 其他情况（如局部变量、全局变量）需要加载
    return true;
}

/// @brief 常量表达式求值
/// @param node AST节点
/// @param result 求值结果
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::evaluate_const_expr(ast_node * node, Value *& result)
{
    if (!node) {
        return false;
    }

    switch (node->node_type) {
        case ast_operator_type::AST_OP_LEAF_LITERAL_UINT:
            // 整数字面量
            result = module->newConstInt(node->integer_val);
            return true;

        case ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT:
            // 浮点数字面量
            result = module->newConstFloat(node->float_val);
            return true;

        case ast_operator_type::AST_OP_ARRAY_ACCESS: {
            // 数组访问：尝试求值为常量
            if (node->sons.size() != 2) {
                return false;
            }

            ast_node * arrayNode = node->sons[0];
            ast_node * indexNode = node->sons[1];

            // 检查数组是否是全局常量
            if (arrayNode->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
                // 查找全局常量数组
                Value * arrayVar = module->findVarValue(arrayNode->name);
                GlobalVariable * globalArray = dynamic_cast<GlobalVariable *>(arrayVar);

                if (globalArray && globalArray->getConstant()) {
                    // 求值索引
                    Value * indexResult = nullptr;
                    if (!evaluate_const_expr(indexNode, indexResult)) {
                        return false;
                    }

                    ConstInt * constIndex = dynamic_cast<ConstInt *>(indexResult);
                    if (!constIndex) {
                        return false;
                    }

                    int index = constIndex->getVal();
                    const std::vector<Value *> & initValues = globalArray->getInitValueList();

                    if (index >= 0 && index < static_cast<int>(initValues.size())) {
                        ConstInt * constValue = dynamic_cast<ConstInt *>(initValues[index]);
                        if (constValue) {
                            result = constValue;
                            // printf("Debug: Evaluated %s[%d] = %d\n",
                            // arrayNode->name.c_str(),
                            // index,
                            // constValue->getVal());
                            return true;
                        }
                    }
                }
            } else if (arrayNode->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
                // 嵌套数组访问，如 c[2][1]
                // printf("Debug: Evaluating nested array access\n");

                // 递归计算基础数组和所有索引
                std::vector<int> indices;
                ast_node * currentNode = node;

                // 收集所有索引（从最内层到最外层）
                while (currentNode && currentNode->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
                    if (currentNode->sons.size() != 2) {
                        return false;
                    }

                    ast_node * indexNode = currentNode->sons[1];
                    Value * indexResult = nullptr;
                    if (!evaluate_const_expr(indexNode, indexResult)) {
                        return false;
                    }

                    ConstInt * constIndex = dynamic_cast<ConstInt *>(indexResult);
                    if (!constIndex) {
                        return false;
                    }

                    indices.insert(indices.begin(), constIndex->getVal()); // 插入到前面
                    currentNode = currentNode->sons[0];
                }

                // 现在 currentNode 应该是基础数组变量
                if (!currentNode || currentNode->node_type != ast_operator_type::AST_OP_LEAF_VAR_ID) {
                    return false;
                }

                // 查找基础数组 - 先尝试局部变量，再尝试全局变量
                Value * arrayVar = module->findVarValue(currentNode->name);
                GlobalVariable * globalArray = dynamic_cast<GlobalVariable *>(arrayVar);

                // printf("Debug: Looking for array '%s', found: %s, isConstant: %s\n",
                // currentNode->name.c_str(),
                // globalArray ? "yes" : "no",
                //(globalArray && globalArray->getConstant()) ? "yes" : "no");

                if (globalArray && globalArray->getConstant()) {
                    // 计算多维数组的线性索引
                    const ArrayType * arrayType = dynamic_cast<const ArrayType *>(globalArray->getType());
                    if (!arrayType) {
                        return false;
                    }

                    // 获取数组维度
                    std::vector<int> dimensions;
                    const Type * currentType = arrayType;
                    while (currentType && currentType->isArrayType()) {
                        const ArrayType * currentArrayType = static_cast<const ArrayType *>(currentType);
                        const std::vector<int> & dims = currentArrayType->getDimensions();
                        if (!dims.empty()) {
                            dimensions.push_back(dims[0]);
                        }
                        currentType = currentArrayType->getElementType();
                    }

                    // 计算线性索引
                    int linearIndex = 0;
                    int multiplier = 1;

                    // 从最后一个维度开始计算
                    for (int i = dimensions.size() - 1; i >= 0; i--) {
                        if (i < static_cast<int>(indices.size())) {
                            linearIndex += indices[i] * multiplier;
                        }
                        multiplier *= dimensions[i];
                    }

                    const std::vector<Value *> & initValues = globalArray->getInitValueList();
                    if (linearIndex >= 0 && linearIndex < static_cast<int>(initValues.size())) {
                        ConstInt * constValue = dynamic_cast<ConstInt *>(initValues[linearIndex]);
                        if (constValue) {
                            result = constValue;
                            // printf("Debug: Evaluated to constant value: %d\n", constValue->getVal());
                            return true;
                        }
                    }
                }

                return false;
            }

            return false;
        }

        case ast_operator_type::AST_OP_LEAF_VAR_ID: {
            // 变量引用：查找全局常量
            Value * varValue = module->findVarValue(node->name);
            GlobalVariable * globalVar = dynamic_cast<GlobalVariable *>(varValue);

            if (globalVar && globalVar->getConstant()) {
                // 这是一个全局常量，获取其初值
                Value * initValue = globalVar->getInitValue();
                if (initValue) {
                    result = initValue;
                    // printf("Debug: Evaluated global constant '%s' to value\n", node->name.c_str());
                    return true;
                }
            }

            // printf("Debug: Variable '%s' is not a global constant\n", node->name.c_str());
            return false;
        }

        case ast_operator_type::AST_OP_ADD: {
            // 加法运算：递归计算操作数
            if (node->sons.size() != 2) {
                return false;
            }

            Value * leftResult = nullptr;
            Value * rightResult = nullptr;

            if (!evaluate_const_expr(node->sons[0], leftResult) || !evaluate_const_expr(node->sons[1], rightResult)) {
                return false;
            }

            // 执行常量折叠
            ConstInt * leftConstInt = dynamic_cast<ConstInt *>(leftResult);
            ConstInt * rightConstInt = dynamic_cast<ConstInt *>(rightResult);
            ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(leftResult);
            ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(rightResult);

            if ((leftConstInt || leftConstFloat) && (rightConstInt || rightConstFloat)) {
                if (leftConstFloat || rightConstFloat) {
                    // 浮点数运算
                    float leftVal =
                        leftConstFloat ? leftConstFloat->getVal() : static_cast<float>(leftConstInt->getVal());
                    float rightVal =
                        rightConstFloat ? rightConstFloat->getVal() : static_cast<float>(rightConstInt->getVal());
                    result = module->newConstFloat(leftVal + rightVal);
                } else {
                    // 整数运算
                    int32_t leftVal = leftConstInt->getVal();
                    int32_t rightVal = rightConstInt->getVal();
                    result = module->newConstInt(leftVal + rightVal);
                }
                return true;
            }

            return false;
        }

        case ast_operator_type::AST_OP_SUB: {
            // 减法运算：递归计算操作数
            if (node->sons.size() != 2) {
                return false;
            }

            Value * leftResult = nullptr;
            Value * rightResult = nullptr;

            if (!evaluate_const_expr(node->sons[0], leftResult) || !evaluate_const_expr(node->sons[1], rightResult)) {
                return false;
            }

            // 执行常量折叠
            ConstInt * leftConstInt = dynamic_cast<ConstInt *>(leftResult);
            ConstInt * rightConstInt = dynamic_cast<ConstInt *>(rightResult);
            ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(leftResult);
            ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(rightResult);

            if ((leftConstInt || leftConstFloat) && (rightConstInt || rightConstFloat)) {
                if (leftConstFloat || rightConstFloat) {
                    // 浮点数运算
                    float leftVal =
                        leftConstFloat ? leftConstFloat->getVal() : static_cast<float>(leftConstInt->getVal());
                    float rightVal =
                        rightConstFloat ? rightConstFloat->getVal() : static_cast<float>(rightConstInt->getVal());
                    result = module->newConstFloat(leftVal - rightVal);
                } else {
                    // 整数运算
                    int32_t leftVal = leftConstInt->getVal();
                    int32_t rightVal = rightConstInt->getVal();
                    result = module->newConstInt(leftVal - rightVal);
                }
                return true;
            }

            return false;
        }

        case ast_operator_type::AST_OP_MUL: {
            // 乘法运算：递归计算操作数
            if (node->sons.size() != 2) {
                return false;
            }

            Value * leftResult = nullptr;
            Value * rightResult = nullptr;

            if (!evaluate_const_expr(node->sons[0], leftResult) || !evaluate_const_expr(node->sons[1], rightResult)) {
                return false;
            }

            // 执行常量折叠
            ConstInt * leftConstInt = dynamic_cast<ConstInt *>(leftResult);
            ConstInt * rightConstInt = dynamic_cast<ConstInt *>(rightResult);
            ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(leftResult);
            ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(rightResult);

            if ((leftConstInt || leftConstFloat) && (rightConstInt || rightConstFloat)) {
                if (leftConstFloat || rightConstFloat) {
                    // 浮点数运算
                    float leftVal =
                        leftConstFloat ? leftConstFloat->getVal() : static_cast<float>(leftConstInt->getVal());
                    float rightVal =
                        rightConstFloat ? rightConstFloat->getVal() : static_cast<float>(rightConstInt->getVal());
                    result = module->newConstFloat(leftVal * rightVal);
                } else {
                    // 整数运算
                    int32_t leftVal = leftConstInt->getVal();
                    int32_t rightVal = rightConstInt->getVal();
                    result = module->newConstInt(leftVal * rightVal);
                }
                return true;
            }

            return false;
        }

        case ast_operator_type::AST_OP_DIV: {
            // 除法运算：递归计算操作数
            if (node->sons.size() != 2) {
                return false;
            }

            Value * leftResult = nullptr;
            Value * rightResult = nullptr;

            if (!evaluate_const_expr(node->sons[0], leftResult) || !evaluate_const_expr(node->sons[1], rightResult)) {
                return false;
            }

            // 执行常量折叠
            ConstInt * leftConstInt = dynamic_cast<ConstInt *>(leftResult);
            ConstInt * rightConstInt = dynamic_cast<ConstInt *>(rightResult);
            ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(leftResult);
            ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(rightResult);

            if ((leftConstInt || leftConstFloat) && (rightConstInt || rightConstFloat)) {
                // 检查除零
                if ((rightConstInt && rightConstInt->getVal() == 0) ||
                    (rightConstFloat && rightConstFloat->getVal() == 0.0f)) {
                    printf("Error: Division by zero in constant expression\n");
                    return false;
                }

                if (leftConstFloat || rightConstFloat) {
                    // 浮点数运算
                    float leftVal =
                        leftConstFloat ? leftConstFloat->getVal() : static_cast<float>(leftConstInt->getVal());
                    float rightVal =
                        rightConstFloat ? rightConstFloat->getVal() : static_cast<float>(rightConstInt->getVal());
                    result = module->newConstFloat(leftVal / rightVal);
                } else {
                    // 整数运算
                    int32_t leftVal = leftConstInt->getVal();
                    int32_t rightVal = rightConstInt->getVal();
                    result = module->newConstInt(leftVal / rightVal);
                }
                return true;
            }

            return false;
        }

        case ast_operator_type::AST_OP_NEGATIVE: {
            // 负号运算：递归计算操作数
            if (node->sons.size() != 1) {
                return false;
            }

            Value * operandResult = nullptr;
            if (!evaluate_const_expr(node->sons[0], operandResult)) {
                return false;
            }

            ConstInt * constInt = dynamic_cast<ConstInt *>(operandResult);
            ConstFloat * constFloat = dynamic_cast<ConstFloat *>(operandResult);

            if (constFloat) {
                result = module->newConstFloat(-constFloat->getVal());
                return true;
            } else if (constInt) {
                result = module->newConstInt(-constInt->getVal());
                return true;
            }

            return false;
        }

        case ast_operator_type::AST_OP_POSITIVE: {
            // 正号运算：递归计算操作数
            if (node->sons.size() != 1) {
                return false;
            }

            Value * operandResult = nullptr;
            if (!evaluate_const_expr(node->sons[0], operandResult)) {
                return false;
            }

            result = operandResult; // 正号不改变值
            return true;
        }

        default:
            // 其他类型的表达式暂不支持
            // printf("Debug: Unsupported expression type %d in evaluate_const_expr\n", (int) node->node_type);
            return false;
    }
}

// filepath: [IRGenerator.cpp](http://_vscodecontentref_/0)
// 在IRGenerator类中添加辅助函数

/// @brief 将值转换为i1类型（如果需要）
/// @param val 输入值
/// @param func 当前函数
/// @param blockInsts 指令块
/// @return i1类型的值
Value * IRGenerator::convertToI1(Value * val, Function * func, InterCode & blockInsts)
{
    // 如果已经是i1类型，直接返回
    if (val->getType()->isIntegerType()) {
        IntegerType * intType = static_cast<IntegerType *>(val->getType());
        if (intType->getBitWidth() == 1) {
            return val;
        }
    }

    // 根据输入值的类型选择合适的零常量
    Value * zeroConst = nullptr;
    if (val->getType()->isFloatType()) {
        // 浮点数类型，使用浮点数零常量
        zeroConst = module->newConstFloat(0.0f);
    } else {
        // 整数类型，使用整数零常量
        zeroConst = module->newConstInt(0);
    }

    RelInstruction * toBoolInst =
        new RelInstruction(func, IRInstOperator::IRINST_OP_NE, val, zeroConst, IntegerType::getTypeBool());
    blockInsts.addInst(toBoolInst);
    return toBoolInst;
}

/// @brief 将i1类型的值扩展为i32类型（如果需要）
/// @param val 输入值
/// @param func 当前函数
/// @param blockInsts 指令块
/// @return i32类型的值
Value * IRGenerator::convertToI32(Value * val, Function * func, InterCode & blockInsts)
{
    // 如果已经是i32类型，直接返回
    if (val->getType()->isIntegerType()) {
        IntegerType * intType = static_cast<IntegerType *>(val->getType());
        if (intType->getBitWidth() == 32) {
            return val;
        }
    }

    // 如果是i1类型，扩展为i32
    ZextInstruction * zextInst = new ZextInstruction(func, val, IntegerType::getTypeInt());
    blockInsts.addInst(zextInst);
    return zextInst;
}

/// @brief 逻辑与运算符AST节点翻译成线性中间IR（条件跳转版本）
/// @param node AST节点
/// @param trueLabel 真出口标签
/// @param falseLabel 假出口标签
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_and_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    ast_node * leftNode = node->sons[0];  // BoolExpr0
    ast_node * rightNode = node->sons[1]; // BoolExpr1

    Function * currentFunc = module->getCurrentFunction();

    // 创建中间标签L3
    LabelInstruction * rightLabel = new LabelInstruction(currentFunc, module); // L3

    // 遍历BoolExpr0生成线性IR后插入（本步需要L3和L2，分别作为条件表达式的真出口和假出口）
    if (!ir_condition_expr(leftNode, rightLabel, falseLabel)) {
        return false;
    }
    node->blockInsts.addInst(leftNode->blockInsts);

    // 然后插入Label指令L3
    node->blockInsts.addInst(rightLabel);

    // 然后遍历BoolExpr1生成线性IR后插入（本步需要L1和L2，分别作为条件表达式的真出口和假出口）
    if (!ir_condition_expr(rightNode, trueLabel, falseLabel)) {
        return false;
    }
    node->blockInsts.addInst(rightNode->blockInsts);

    // 条件表达式不设置val值
    node->val = nullptr;

    return true;
}

/// @brief 逻辑或运算符AST节点翻译成线性中间IR（条件跳转版本）
/// @param node AST节点
/// @param trueLabel 真出口标签
/// @param falseLabel 假出口标签
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_or_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    ast_node * leftNode = node->sons[0];  // BoolExpr0
    ast_node * rightNode = node->sons[1]; // BoolExpr1

    Function * currentFunc = module->getCurrentFunction();

    // 创建中间标签L3
    LabelInstruction * rightLabel = new LabelInstruction(currentFunc, module); // L3

    // 遍历BoolExpr0生成线性IR后插入（本步需要L1和L3，分别作为条件表达式的真出口和假出口）
    if (!ir_condition_expr(leftNode, trueLabel, rightLabel)) {
        return false;
    }
    node->blockInsts.addInst(leftNode->blockInsts);

    // 然后插入Label指令L3
    node->blockInsts.addInst(rightLabel);

    // 然后遍历BoolExpr1生成线性IR后插入（本步需要L1和L2，分别作为条件表达式的真出口和假出口）
    if (!ir_condition_expr(rightNode, trueLabel, falseLabel)) {
        return false;
    }
    node->blockInsts.addInst(rightNode->blockInsts);

    // 条件表达式不设置val值
    node->val = nullptr;

    return true;
}

/// @brief 逻辑非运算符AST节点翻译成线性中间IR（条件跳转版本）
/// @param node AST节点
/// @param trueLabel 真出口标签
/// @param falseLabel 假出口标签
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_not_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    ast_node * operandNode = node->sons[0];

    // 遍历子树生成线性IR后插入，本步需要L2和L1，分别作为条件表达式的真出口和假出口（注意这里，L2对应的是真出口）
    if (!ir_condition_expr(operandNode, falseLabel, trueLabel)) {
        return false;
    }
    node->blockInsts.addInst(operandNode->blockInsts);

    // 条件表达式不设置val值
    node->val = nullptr;

    return true;
}

/// @brief 关系表达式AST节点翻译成线性中间IR（条件跳转版本）
/// @param node AST节点
/// @param trueLabel 真出口标签
/// @param falseLabel 假出口标签
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_rel_exp_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    // 确保节点有两个子节点：左操作数和右操作数
    if (node->sons.size() != 2) {
        printf("Error: Invalid node structure in ir_rel_exp_with_labels.\n");
        return false;
    }

    // 获取左操作数和右操作数
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    // 递归生成左操作数和右操作数的 IR
    if (!ir_visit_ast_node(leftNode) || !ir_visit_ast_node(rightNode)) {
        return false;
    }

    // 获取操作符类型
    IRInstOperator op;
    switch (node->node_type) {
        case ast_operator_type::AST_OP_LT:
            op = IRInstOperator::IRINST_OP_LT;
            break;
        case ast_operator_type::AST_OP_GT:
            op = IRInstOperator::IRINST_OP_GT;
            break;
        case ast_operator_type::AST_OP_LE:
            op = IRInstOperator::IRINST_OP_LE;
            break;
        case ast_operator_type::AST_OP_GE:
            op = IRInstOperator::IRINST_OP_GE;
            break;
        case ast_operator_type::AST_OP_EQ:
            op = IRInstOperator::IRINST_OP_EQ;
            break;
        case ast_operator_type::AST_OP_NE:
            op = IRInstOperator::IRINST_OP_NE;
            break;
        default:
            printf("Error: Unsupported operator in ir_rel_exp_with_labels.\n");
            return false;
    }

    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 处理左操作数
    if (needsLoad(leftNode->val)) {
        // 左操作数是变量，需要加载
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), leftNode->val, leftNode->val, 4);
        node->blockInsts.addInst(leftNode->blockInsts);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        // 左操作数是常量或表达式结果，直接使用
        node->blockInsts.addInst(leftNode->blockInsts);
        leftValue = leftNode->val;
    }

    // 处理右操作数
    if (needsLoad(rightNode->val)) {
        // 右操作数是变量，需要加载
        LoadInstruction * loadRight =
            new LoadInstruction(module->getCurrentFunction(), rightNode->val, rightNode->val, 4);
        node->blockInsts.addInst(rightNode->blockInsts);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        // 右操作数是常量或表达式结果，直接使用
        node->blockInsts.addInst(rightNode->blockInsts);
        rightValue = rightNode->val;
    }

    // 类型转换：如果任一操作数是浮点数，将另一个操作数也转换为浮点数
    Type * leftType = leftValue->getType();
    Type * rightType = rightValue->getType();

    if (leftType->isFloatType() || rightType->isFloatType()) {
        // 确保两个操作数都是浮点数类型
        if (!leftValue->getType()->isFloatType()) {
            // 特殊处理常量：如果是整数常量，直接创建对应的浮点常量
            if (ConstInt * constInt = dynamic_cast<ConstInt *>(leftValue)) {
                leftValue = module->newConstFloat(static_cast<float>(constInt->getVal()));
            } else {
                leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
                if (!leftValue) {
                    printf("Error: Failed to convert left operand to float type in comparison with labels.\n");
                    return false;
                }
            }
        }
        if (!rightValue->getType()->isFloatType()) {
            // 特殊处理常量：如果是整数常量，直接创建对应的浮点常量
            if (ConstInt * constInt = dynamic_cast<ConstInt *>(rightValue)) {
                rightValue = module->newConstFloat(static_cast<float>(constInt->getVal()));
            } else {
                rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
                if (!rightValue) {
                    printf("Error: Failed to convert right operand to float type in comparison with labels.\n");
                    return false;
                }
            }
        }
    } else {
        // 处理整数和布尔值的比较
        // 如果一个操作数是i1类型，另一个是i32类型，需要进行类型转换
        if (leftType->isInt1Byte() && rightType->isInt32Type()) {
            // 将i1类型转换为i32类型
            leftValue = convertToI32(leftValue, module->getCurrentFunction(), node->blockInsts);
        } else if (leftType->isInt32Type() && rightType->isInt1Byte()) {
            // 将i1类型转换为i32类型
            rightValue = convertToI32(rightValue, module->getCurrentFunction(), node->blockInsts);
        }
    }

    // 创建关系表达式指令
    RelInstruction * relInst =
        new RelInstruction(module->getCurrentFunction(), op, leftValue, rightValue, IntegerType::getTypeBool());
    node->blockInsts.addInst(relInst);

    // 生成条件跳转指令
    BranchInstruction * branchInst =
        new BranchInstruction(module->getCurrentFunction(), relInst, trueLabel, falseLabel);
    node->blockInsts.addInst(branchInst);

    // 条件表达式不设置val值
    node->val = nullptr;

    return true;
}

/// @brief 处理条件表达式，根据节点类型选择合适的处理方法
/// @param node AST节点
/// @param trueLabel 真出口标签
/// @param falseLabel 假出口标签
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_condition_expr(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    switch (node->node_type) {
        case ast_operator_type::AST_OP_AND:
            return ir_and_with_labels(node, trueLabel, falseLabel);

        case ast_operator_type::AST_OP_OR:
            return ir_or_with_labels(node, trueLabel, falseLabel);

        case ast_operator_type::AST_OP_NOT:
            return ir_not_with_labels(node, trueLabel, falseLabel);

        case ast_operator_type::AST_OP_LT:
        case ast_operator_type::AST_OP_GT:
        case ast_operator_type::AST_OP_LE:
        case ast_operator_type::AST_OP_GE:
        case ast_operator_type::AST_OP_EQ:
        case ast_operator_type::AST_OP_NE:
            return ir_rel_exp_with_labels(node, trueLabel, falseLabel);

        default:
            // 对于其他表达式（如变量、常量），需要转换为条件跳转
            if (!ir_visit_ast_node(node)) {
                return false;
            }

            Value * condValue = nullptr;
            if (needsLoad(node->val)) {
                LoadInstruction * loadCond = new LoadInstruction(module->getCurrentFunction(), node->val, node->val, 4);
                node->blockInsts.addInst(loadCond);
                condValue = loadCond;
            } else {
                condValue = node->val;
            }

            // 转换为i1类型并生成条件跳转
            Value * boolValue = convertToI1(condValue, module->getCurrentFunction(), node->blockInsts);
            BranchInstruction * branchInst =
                new BranchInstruction(module->getCurrentFunction(), boolValue, trueLabel, falseLabel);
            node->blockInsts.addInst(branchInst);

            return true;
    }
}

/// @brief 检查指令序列的最后一条指令是否是终结指令（如break、continue、return）
/// @param blockInsts 指令序列
/// @return true：最后一条指令是终结指令，false：不是
bool IRGenerator::hasTerminatorInstruction(const InterCode & blockInsts)
{
    const auto & insts = blockInsts.getCode();
    if (insts.empty()) {
        return false;
    }

    // 检查最后一条指令是否是终结指令
    Instruction * lastInst = insts.back();
    IRInstOperator op = lastInst->getOp();

    return (op == IRInstOperator::IRINST_OP_GOTO ||  // break, continue, 无条件跳转
            op == IRInstOperator::IRINST_OP_RET ||   // return
            op == IRInstOperator::IRINST_OP_BRANCH); // 条件跳转
}

/// @brief 检查指令序列是否包含return指令
/// @param blockInsts 指令序列
/// @return true：包含return指令，false：不包含
bool IRGenerator::hasReturnInstruction(const InterCode & blockInsts)
{
    const auto & insts = blockInsts.getCode();
    if (insts.empty()) {
        return false;
    }

    // 检查最后一条指令是否是return指令
    Instruction * lastInst = insts.back();
    IRInstOperator op = lastInst->getOp();

    return (op == IRInstOperator::IRINST_OP_RET); // 只检查return指令
}

/// @brief 常量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_const_declare_statement(ast_node * node)
{
    bool result = true;

    for (auto & child: node->sons) {
        // 遍历每个常量声明
        result = ir_const_declare(child);
        if (!result) {
            break;
        }
        // 收集子节点生成的 IR
        node->blockInsts.addInst(child->blockInsts);
    }

    return result;
}

/// @brief 常量声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_const_declare(ast_node * node)
{
    // 常量声明的AST包含两个孩子：
    // 第一个孩子：类型节点
    // 第二个孩子：赋值节点（包含常量名和初值）

    if (node->sons.size() != 2) {
        printf("Error: Invalid constant declaration structure.\n");
        return false;
    }

    ast_node * typeNode = node->sons[0];
    ast_node * assignNode = node->sons[1];

    if (assignNode->node_type != ast_operator_type::AST_OP_ASSIGN) {
        printf("Error: Constant declaration must have initialization.\n");
        return false;
    }

    ast_node * nameNode = assignNode->sons[0];
    ast_node * initExprNode = assignNode->sons[1];

    // 检查当前是否在全局作用域
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        // 全局常量处理
        return ir_global_const_declare(node, typeNode, nameNode, initExprNode);
    } else {
        // 局部常量处理
        return ir_local_const_declare(node, typeNode, nameNode, initExprNode);
    }
}

/// @brief 全局常量声明节点翻译成线性中间IR
/// @param node AST节点
/// @param typeNode 类型节点
/// @param nameNode 常量名节点
/// @param initExprNode 初值表达式节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_global_const_declare(ast_node * node,
                                          ast_node * typeNode,
                                          ast_node * nameNode,
                                          ast_node * initExprNode)
{
    // 检查初始化表达式类型
    if (initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT ||
        initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
        // 处理标量字面量（整数或浮点数）
        return ir_global_const_scalar_declare(node, typeNode, nameNode, initExprNode);
    } else if (initExprNode->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
        // 处理数组初始化
        return ir_global_const_array_declare(node, typeNode, nameNode, initExprNode);
    } else {
        // 尝试计算常量表达式
        Value * constValue = nullptr;
        if (evaluate_const_expr(initExprNode, constValue)) {
            // 成功计算出常量值，创建一个临时的字面量节点
            ast_node * tempLiteralNode = ast_node::New("", initExprNode->line_no);

            // 根据目标类型进行适当的类型转换
            if (typeNode->type->isIntegerType()) {
                // 目标是整数类型
                if (ConstInt * constInt = dynamic_cast<ConstInt *>(constValue)) {
                    tempLiteralNode->node_type = ast_operator_type::AST_OP_LEAF_LITERAL_UINT;
                    tempLiteralNode->integer_val = constInt->getVal();
                } else if (ConstFloat * constFloat = dynamic_cast<ConstFloat *>(constValue)) {
                    // 浮点数转整数
                    tempLiteralNode->node_type = ast_operator_type::AST_OP_LEAF_LITERAL_UINT;
                    tempLiteralNode->integer_val = static_cast<int32_t>(constFloat->getVal());
                    // printf("Debug: Converting float constant %f to integer %d\n",
                    // constFloat->getVal(),
                    // tempLiteralNode->integer_val);
                } else {
                    printf("Error: Constant expression evaluation returned non-constant value.\n");
                    return false;
                }
            } else if (typeNode->type->isFloatType()) {
                // 目标是浮点数类型
                if (ConstFloat * constFloat = dynamic_cast<ConstFloat *>(constValue)) {
                    tempLiteralNode->node_type = ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT;
                    tempLiteralNode->float_val = constFloat->getVal();
                } else if (ConstInt * constInt = dynamic_cast<ConstInt *>(constValue)) {
                    // 整数转浮点数
                    tempLiteralNode->node_type = ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT;
                    tempLiteralNode->float_val = static_cast<float>(constInt->getVal());
                    // printf("Debug: Converting integer constant %d to float %f\n",
                    // constInt->getVal(),
                    // tempLiteralNode->float_val);
                } else {
                    printf("Error: Constant expression evaluation returned non-constant value.\n");
                    return false;
                }
            } else {
                printf("Error: Unsupported target type for constant expression.\n");
                return false;
            }

            // 使用临时字面量节点进行标量声明
            bool result = ir_global_const_scalar_declare(node, typeNode, nameNode, tempLiteralNode);

            // 清理临时节点
            delete tempLiteralNode;

            return result;
        } else {
            printf("Error: Global constant must be initialized with literal value, array initializer, or constant "
                   "expression.\n");
            return false;
        }
    }
}

/// @brief 全局常量标量声明节点翻译成线性中间IR
/// @param node AST节点
/// @param typeNode 类型节点
/// @param nameNode 常量名节点
/// @param initExprNode 初值表达式节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_global_const_scalar_declare(ast_node * node,
                                                 ast_node * typeNode,
                                                 ast_node * nameNode,
                                                 ast_node * initExprNode)
{
    Value * constValue = nullptr;

    // printf("Debug: Processing global constant '%s' with init node type: %d\n",
    // nameNode->name.c_str(),
    // static_cast<int>(initExprNode->node_type));

    // 根据初始化表达式的类型创建相应的常量值
    if (initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
        // 整数字面量 - 直接使用已解析的值
        int32_t intValue = static_cast<int32_t>(initExprNode->integer_val);
        // printf("Debug: Processing integer literal with value: %d\n", intValue);

        if (typeNode->type->isFloatType()) {
            // 目标类型是浮点数，进行类型转换
            constValue = module->newConstFloat(static_cast<float>(intValue));
            // printf("Debug: Converting integer %d to float %f\n", intValue, static_cast<float>(intValue));
        } else {
            // 目标类型是整数
            constValue = module->newConstInt(intValue);
        }
    } else if (initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
        // 浮点数字面量
        float floatValue = initExprNode->float_val;
        // printf("Debug: Processing float literal with value: %f\n", floatValue);

        if (typeNode->type->isIntegerType()) {
            // 目标类型是整数，进行类型转换
            constValue = module->newConstInt(static_cast<int32_t>(floatValue));
        } else {
            // 目标类型是浮点数
            constValue = module->newConstFloat(floatValue);
        }
    } else {
        printf("Error: Unsupported constant type for global constant declaration.\n");
        return false;
    }

    if (!constValue) {
        printf("Error: Failed to create constant value.\n");
        return false;
    }

    // 创建全局常量变量
    Value * globalConst = module->newVarValue(typeNode->type, nameNode->name);
    if (!globalConst) {
        printf("Error: Failed to create global constant '%s'.\n", nameNode->name.c_str());
        return false;
    }

    // 将其转换为 GlobalVariable 并设置初值
    GlobalVariable * globalVar = static_cast<GlobalVariable *>(globalConst);
    globalVar->setInitValue(constValue);

    // 设置BSS段标志
    if (ConstInt * constInt = dynamic_cast<ConstInt *>(constValue)) {
        if (constInt->getVal() != 0) {
            globalVar->setBSSSection(false);
        }
    } else if (ConstFloat * constFloat = dynamic_cast<ConstFloat *>(constValue)) {
        if (constFloat->getVal() != 0.0f) {
            globalVar->setBSSSection(false);
        }
    }

    globalVar->setConstant(true);

    // 设置节点值
    nameNode->val = globalConst;
    node->val = globalConst;

    return true;
}

/// @brief 全局常量数组声明节点翻译成线性中间IR
/// @param node AST节点
/// @param typeNode 类型节点
/// @param nameNode 常量名节点
/// @param initExprNode 初值表达式节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_global_const_array_declare(ast_node * node,
                                                ast_node * typeNode,
                                                ast_node * nameNode,
                                                ast_node * initExprNode)
{
    // 确保类型是数组类型
    if (!typeNode->type->isArrayType()) {
        printf("Error: Expected array type for array initialization.\n");
        return false;
    }

    ArrayType * arrayType = static_cast<ArrayType *>(typeNode->type);

    // 对于常量数组，使用 __const.main.xxx 命名格式，并添加唯一计数器避免重名
    static int constArrayCounter = 0;
    std::string constArrayName = "__const.main." + nameNode->name + "." + std::to_string(constArrayCounter++);
    Value * globalVar = module->newVarValue(arrayType, constArrayName);
    if (!globalVar) {
        printf("Error: Failed to create global constant '%s'.\n", nameNode->name.c_str());
        return false;
    }

    // 转换为 GlobalVariable 并设置属性
    GlobalVariable * globalArray = static_cast<GlobalVariable *>(globalVar);
    globalArray->setConstant(true);
    globalArray->setBSSSection(false); // 有初值，不在BSS段
    globalArray->setAlignment(16);     // 设置16字节对齐

    // 直接处理初始化值，不调用ir_array_init以避免重复创建
    std::vector<Value *> initValues;

    // 递归处理数组初始化
    if (!processArrayInitialization(initExprNode, arrayType, initValues)) {
        printf("Error: Failed to process array initialization for global constant '%s'.\n", nameNode->name.c_str());
        return false;
    }

    // printf("Debug: Processed %zu initialization values for global constant '%s'\n",
    // initValues.size(),
    // nameNode->name.c_str());

    // 设置初始化值列表
    globalArray->setInitValueList(initValues);

    // 设置节点值
    nameNode->val = globalArray;
    node->val = globalArray;

    // 重要：在全局作用域中注册原始名称的别名，使得函数内部可以通过原始名称找到这个全局常量数组
    // 使用newVarValue方法创建别名，这会自动注册到符号表中
    Value * aliasVar = module->newVarValue(arrayType, nameNode->name);
    if (aliasVar) {
        GlobalVariable * aliasGlobalVar = static_cast<GlobalVariable *>(aliasVar);
        aliasGlobalVar->setConstant(true);
        aliasGlobalVar->setBSSSection(false);
        aliasGlobalVar->setAlignment(16);
        aliasGlobalVar->setInitValueList(globalArray->getInitValueList());
        // printf("Debug: Created alias variable '%s' for global constant array\n", nameNode->name.c_str());
    }

    // 收集生成的IR指令
    node->blockInsts.addInst(initExprNode->blockInsts);

    return true;
}

/// @brief 局部常量声明节点翻译成线性中间IR
/// @param node AST节点
/// @param typeNode 类型节点
/// @param nameNode 常量名节点
/// @param initExprNode 初值表达式节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_local_const_declare(ast_node * node,
                                         ast_node * typeNode,
                                         ast_node * nameNode,
                                         ast_node * initExprNode)
{
    // 计算初值表达式
    if (!ir_visit_ast_node(initExprNode)) {
        printf("Error: Failed to evaluate constant initialization expression.\n");
        return false;
    }

    Value * initValue = nullptr;

    // 处理初始值
    if (needsLoad(initExprNode->val)) {
        // 如果初值是变量，需要加载
        LoadInstruction * loadInit =
            new LoadInstruction(module->getCurrentFunction(), initExprNode->val, initExprNode->val, 4);
        node->blockInsts.addInst(initExprNode->blockInsts);
        node->blockInsts.addInst(loadInit);
        initValue = loadInit;
    } else {
        // 初始值是常量或表达式结果
        node->blockInsts.addInst(initExprNode->blockInsts);
        initValue = initExprNode->val;
    }

    // 局部常量按照只读变量处理（生成 alloca + store）
    // 常量变量的类型应该是指向常量类型的指针类型
    Type * constType = typeNode->type;                // 常量类型
    Type * constVarType = new PointerType(constType); // 常量变量类型（指针类型）
    Value * constVar = module->newVarValue(constVarType, nameNode->name);
    if (!constVar) {
        printf("Error: Failed to create constant variable '%s'.\n", nameNode->name.c_str());
        return false;
    }

    // 创建alloca指令
    AllocaInstruction * allocaInst = new AllocaInstruction(module->getCurrentFunction(), constVar, constType, 4);
    node->blockInsts.addInst(allocaInst);

    // 创建store指令
    StoreInstruction * storeInst = new StoreInstruction(module->getCurrentFunction(), initValue, constVar, 4);
    node->blockInsts.addInst(storeInst);

    nameNode->val = constVar;
    node->val = constVar;

    return true;
}

/// @brief 数组访问AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_access(ast_node * node)
{
    if (node->sons.size() != 2) {
        printf("Error: Invalid array access node structure. Expected 2 children, got %zu.\n", node->sons.size());
        return false;
    }

    ast_node * arrayNode = node->sons[0];
    ast_node * indexNode = node->sons[1];

    Value * arrayVar = nullptr;

    // 处理数组基址
    if (arrayNode->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        // 优先使用节点的val（可能是更新后的动态数组变量）
        if (arrayNode->val) {
            arrayVar = arrayNode->val;
            // printf("Debug: Using arrayNode->val for variable %s\n", arrayNode->name.c_str());
        } else {
            arrayVar = module->findVarValue(arrayNode->name);
            if (!arrayVar) {
                printf("Error: Array variable %s not found.\n", arrayNode->name.c_str());
                return false;
            }
            // printf("Debug: Using symbol table lookup for variable %s\n", arrayNode->name.c_str());

            // 检查是否是动态数组，如果是则需要查找更新后的变量
            if (arrayVar->getType()->isArrayType()) {
                ArrayType * arrayType = static_cast<ArrayType *>(arrayVar->getType());
                const std::vector<int> & dimensions = arrayType->getDimensions();

                // 如果包含-1维度，说明这是原始的动态数组变量，需要查找更新后的变量
                for (int dim: dimensions) {
                    if (dim == -1) {
                        // printf("Debug: Found dynamic array %s with -1 dimension, looking for updated variable\n",
                        // arrayNode->name.c_str());

                        // 查找带有_ACTUAL_SIZE_后缀的变量
                        std::string updatedName = arrayNode->name + "_ACTUAL_SIZE_32"; // 假设是32字节

                        // 尝试查找更新后的变量
                        Value * updatedVar = module->findVarValue(updatedName);
                        if (updatedVar) {
                            // printf("Debug: Found updated dynamic array variable: %s\n", updatedName.c_str());
                            arrayVar = updatedVar;
                        } else {
                            // printf("Debug: Could not find updated variable %s, using original\n",
                            // updatedName.c_str());
                        }
                        break;
                    }
                }
            }
        }

        // 检查是否是函数参数（指针类型），如果是则需要先加载
        // 对于函数参数，变量类型是指向指针的指针（如i32**），需要加载得到真正的数组指针
        if (arrayVar->getType()->isPointerType()) {
            const PointerType * ptrType = static_cast<const PointerType *>(arrayVar->getType());
            // 如果指向的是指针类型（即函数参数），需要加载
            if (ptrType->getPointeeType()->isPointerType()) {
                LoadInstruction * loadPtr = new LoadInstruction(module->getCurrentFunction(), arrayVar, arrayVar, 8);
                node->blockInsts.addInst(loadPtr);
                arrayVar = loadPtr;
            }
        }
    } else if (arrayNode->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
        // 嵌套数组访问：如 matrix[0][1] 中的 matrix[0] 部分
        if (!ir_visit_ast_node(arrayNode)) {
            return false;
        }
        node->blockInsts.addInst(arrayNode->blockInsts);
        arrayVar = arrayNode->val; // 这是一个指向子数组的指针
    } else {
        if (!ir_visit_ast_node(arrayNode)) {
            return false;
        }
        node->blockInsts.addInst(arrayNode->blockInsts);
        arrayVar = arrayNode->val;
    }

    // 处理索引表达式
    Value * indexValue = nullptr;

    // 处理索引表达式
    if (!ir_visit_ast_node(indexNode)) {
        return false;
    }
    node->blockInsts.addInst(indexNode->blockInsts);

    if (needsLoad(indexNode->val)) {
        LoadInstruction * loadIndex =
            new LoadInstruction(module->getCurrentFunction(), indexNode->val, indexNode->val, 4);
        node->blockInsts.addInst(loadIndex);

        // 确保 SextInstruction 接收的是 i32 类型的值
        if (loadIndex->getType()->isInt32Type()) {
            SextInstruction * sextIndex =
                new SextInstruction(module->getCurrentFunction(), loadIndex, IntegerType::getTypeLong());
            node->blockInsts.addInst(sextIndex);
            indexValue = sextIndex;
        } else {
            printf("Error: Load instruction result type is not i32\n");
            return false;
        }
    } else {
        // 对于常量和其他情况，都需要扩展到64位
        ConstInt * constIndex = dynamic_cast<ConstInt *>(indexNode->val);
        if (constIndex) {
            // 直接创建64位常量，而不是使用sext
            int64_t constVal = constIndex->getLongVal();          // 获取64位值
            ConstInt * i64Const = module->newConstLong(constVal); // 创建64位常量
            indexValue = i64Const;
        } else {
            // 其他情况（如表达式结果），需要扩展
            SextInstruction * sextIndex =
                new SextInstruction(module->getCurrentFunction(), indexNode->val, IntegerType::getTypeLong());
            node->blockInsts.addInst(sextIndex);
            indexValue = sextIndex;
        }
    }

    // 生成getelementptr指令
    GetelementptrInstruction * gepInst = nullptr;

    // 检查arrayVar的类型来决定使用哪种getelementptr格式
    if (arrayVar->getType()->isArrayType()) {
        // 对于多维数组类型，需要检查是否是最后一维访问
        const ArrayType * arrayType = static_cast<const ArrayType *>(arrayVar->getType());
        const std::vector<int> & dimensions = arrayType->getDimensions();

        if (dimensions.size() > 1) {
            // 多维数组：第一次访问返回子数组的指针
            ConstInt * zeroConst = module->newConstInt(0);
            gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, zeroConst, indexValue);
        } else {
            // 一维数组：直接访问元素
            ConstInt * zeroConst = module->newConstInt(0);
            gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, zeroConst, indexValue);
        }
    } else if (arrayVar->getType()->isPointerType()) {
        // 对于指针类型（可能是多维数组的中间访问结果）
        const PointerType * ptrType = static_cast<const PointerType *>(arrayVar->getType());
        const Type * pointeeType = ptrType->getPointeeType();

        if (pointeeType->isArrayType()) {
            // 指向数组的指针：需要区分函数参数和中间结果
            bool isFunctionParameter = false;

            // 检查是否是函数参数：通过检查变量名和当前访问模式
            if (arrayNode->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
                // 检查变量是否是函数参数
                Function * currentFunc = module->getCurrentFunction();
                if (currentFunc) {
                    const std::vector<FormalParam *> & params = currentFunc->getParams();
                    for (FormalParam * param: params) {
                        if (param->getName() == arrayNode->name) {
                            isFunctionParameter = true;
                            break;
                        }
                    }
                }
            }

            if (isFunctionParameter) {
                // 函数参数：第一次访问使用单索引
                gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, indexValue);
            } else {
                // 局部变量或中间结果：使用双索引
                ConstInt * zeroConst = module->newConstInt(0);
                gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, zeroConst, indexValue);
            }
        } else {
            // 指向元素的指针：使用单个索引 [index]
            gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, indexValue);
        }
    } else {
        printf("Error: Invalid array base type for getelementptr.\n");
        return false;
    }

    node->blockInsts.addInst(gepInst);

    // 数组访问的结果是地址，不是值
    node->val = gepInst;

    return true;
}

/// @brief 数组初始化AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_init(ast_node * node)
{
    Function * currentFunc = module->getCurrentFunction();

    // 收集初始化值
    std::vector<Value *> initValues;
    Type * elementType = node->type; // 基础元素类型

    // printf("=== ir_array_init: Processing %zu elements, elementType = %s, varName = %s ===\n",
    // node->sons.size(),
    // elementType ? elementType->toString().c_str() : "null",
    // node->name.c_str());

    // 如果elementType是void或null，尝试从上下文推断类型
    if (!elementType || elementType->isVoidType()) {
        // printf("Debug: elementType is void/null, trying to infer from context\n");
        //  对于顶层数组初始化，尝试从父节点获取类型信息
        //  这是一个临时修复，理想情况下类型应该在AST构建时正确设置
        if (node->name.empty() && node->sons.size() > 0) {
            // 这可能是一个嵌套的数组初始化，尝试推断类型
            // printf("Debug: This appears to be a nested array init, using i32 as fallback\n");
            elementType = module->getI32Type();
        }
    }

    // 尝试从父节点获取目标数组类型信息
    ArrayType * targetArrayType = nullptr;
    if (node->parent && node->parent->type && node->parent->type->isArrayType()) {
        targetArrayType = static_cast<ArrayType *>(node->parent->type);
        // printf("Debug: Found target array type from parent: %s\n", targetArrayType->toString().c_str());
    }

    // 确定内层元素的类型
    Type * innerElementType = elementType;
    if (elementType && elementType->isArrayType()) {
        // 如果当前类型是数组类型，获取其元素类型作为内层类型
        const ArrayType * arrType = static_cast<const ArrayType *>(elementType);
        innerElementType = const_cast<Type *>(arrType->getElementType());
        // printf("Debug: Inner element type = %s\n", innerElementType->toString().c_str());
    }

    for (auto son: node->sons) {
        if (son->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
            // 嵌套的数组初始化列表 - 传递正确的数组类型
            if (elementType && elementType->isArrayType()) {
                // 对于多维数组，需要正确推断嵌套初始化的类型
                const ArrayType * elementArrayType = static_cast<const ArrayType *>(elementType);

                // 检查嵌套初始化的内容来决定类型
                // 如果嵌套初始化包含基础类型元素（如数字），则应该使用最内层的数组类型
                bool hasBasicElements = false;
                for (auto grandson: son->sons) {
                    if (grandson->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT ||
                        grandson->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
                        hasBasicElements = true;
                        break;
                    }
                }

                if (hasBasicElements) {
                    // 如果包含基础元素，找到最内层的数组类型
                    const Type * innerMostArrayType = elementType;
                    while (innerMostArrayType->isArrayType()) {
                        const ArrayType * currentArrayType = static_cast<const ArrayType *>(innerMostArrayType);
                        if (currentArrayType->getElementType()->isArrayType()) {
                            innerMostArrayType = currentArrayType->getElementType();
                        } else {
                            // 找到了最内层的数组类型
                            break;
                        }
                    }
                    son->type = const_cast<Type *>(innerMostArrayType);
                    // printf("Debug: Setting nested array init type to %s (innermost for basic elements)\n",
                    // son->type->toString().c_str());
                } else {
                    // 如果不包含基础元素，使用元素类型
                    son->type = const_cast<Type *>(elementArrayType->getElementType());
                    // printf("Debug: Setting nested array init type to %s (element type)\n",
                    // son->type->toString().c_str());
                }
            } else {
                // 如果不是数组类型，使用内层元素类型
                son->type = innerElementType;
                // printf("Debug: Setting nested array init type to %s (fallback)\n",
                // innerElementType->toString().c_str());
            }
        }

        if (!ir_visit_ast_node(son)) {
            return false;
        }
        node->blockInsts.addInst(son->blockInsts);

        Value * initVal = son->val;

        // 对于常量值，不需要load，但需要类型转换
        if (dynamic_cast<ConstInt *>(initVal) || dynamic_cast<ConstFloat *>(initVal)) {
            // 检查是否需要类型转换
            Value * convertedVal = initVal;
            if (innerElementType) {
                if (innerElementType->isFloatType() && !initVal->getType()->isFloatType()) {
                    // 目标是浮点数，源是整数，需要转换
                    ConstInt * constInt = dynamic_cast<ConstInt *>(initVal);
                    if (constInt) {
                        convertedVal = module->newConstFloat(static_cast<float>(constInt->getVal()));
                        // printf("Debug: Converting array init value from int %d to float %f\n",
                        // constInt->getVal(),
                        // static_cast<float>(constInt->getVal()));
                    }
                } else if (innerElementType->isIntegerType() && !initVal->getType()->isIntegerType()) {
                    // 目标是整数，源是浮点数，需要转换
                    ConstFloat * constFloat = dynamic_cast<ConstFloat *>(initVal);
                    if (constFloat) {
                        convertedVal = module->newConstInt(static_cast<int32_t>(constFloat->getVal()));
                        // printf("Debug: Converting array init value from float %f to int %d\n",
                        // constFloat->getVal(),
                        // static_cast<int32_t>(constFloat->getVal()));
                    }
                }
            }
            initValues.push_back(convertedVal);
        } else if (dynamic_cast<GlobalVariable *>(initVal)) {
            // 这是嵌套数组的全局常量，需要展开其初始化值
            GlobalVariable * nestedArray = static_cast<GlobalVariable *>(initVal);
            const std::vector<Value *> & nestedValues = nestedArray->getInitValueList();

            // printf("Debug: Found nested GlobalVariable '%s' with %zu values\n",
            // nestedArray->getIRName().c_str(),
            // nestedValues.size());

            if (nestedValues.empty()) {
                // printf("Debug: Nested array has no init values, treating as single element\n");
                initValues.push_back(initVal);
            } else {
                // printf("Debug: Expanding nested array with %zu values\n", nestedValues.size());
                for (auto nestedVal: nestedValues) {
                    initValues.push_back(nestedVal);
                    if (auto constInt = dynamic_cast<ConstInt *>(nestedVal)) {
                        // printf("Debug: Expanded value: %d\n", constInt->getVal());
                    }
                }
            }
        } else if (son->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
            // 对于数组访问表达式，尝试在编译时求值
            Value * constResult = nullptr;
            if (evaluate_const_expr(son, constResult)) {
                // printf("Debug: Successfully evaluated array access to constant: %d\n",
                // dynamic_cast<ConstInt *>(constResult)->getVal());
                initValues.push_back(constResult);
            } else {
                // 无法在编译时求值，暂时跳过，让调用者处理动态初始化
                // printf("Debug: Array access cannot be evaluated at compile time\n");
            }
        } else if (needsLoad(initVal)) {
            LoadInstruction * loadInst = new LoadInstruction(currentFunc, initVal, initVal, 4);
            node->blockInsts.addInst(loadInst);
            initValues.push_back(loadInst);
        } else {
            initValues.push_back(initVal);
        }
    }

    // printf("=== ir_array_init: Collected %zu init values for array %s ===\n", initValues.size(), node->name.c_str());

    // 确定数组类型
    ArrayType * arrayType = nullptr;
    bool isFlatInitialization = false;
    bool isMixedInitialization = false;

    // 首先检查当前节点是否已经有正确的数组类型信息（由父节点设置）
    if (node->type && node->type->isArrayType()) {
        // 使用当前节点的数组类型（已由handleStaticInitialization设置）
        arrayType = static_cast<ArrayType *>(node->type);
        // printf("Debug: Using node-provided array type: %s\n", arrayType->toString().c_str());

        // 对于多维数组，计算真正的总元素个数
        int expectedElements = arrayType->getTotalElements();
        int realTotalElements = 1;

        // 递归计算所有嵌套维度的总元素个数
        const Type * currentType = arrayType;
        while (currentType && currentType->isArrayType()) {
            const ArrayType * currentArrayType = static_cast<const ArrayType *>(currentType);
            const std::vector<int> & dims = currentArrayType->getDimensions();
            if (!dims.empty()) {
                realTotalElements *= dims[0];
            }
            currentType = currentArrayType->getElementType();
        }

        // printf("Debug: Array expects %d elements (outer), %d total elements, got %zu init values\n",
        // expectedElements,
        // realTotalElements,
        // initValues.size());

        // 检查是否是扁平化初始化（所有值都是基础常量且数量等于真正的总元素数）
        if (arrayType->getElementType()->isArrayType()) {
            bool allBasicConstants = true;
            for (Value * val: initValues) {
                if (!dynamic_cast<ConstInt *>(val) && !dynamic_cast<ConstFloat *>(val)) {
                    allBasicConstants = false;
                    break;
                }
            }
            if (allBasicConstants && initValues.size() <= static_cast<size_t>(realTotalElements)) {
                isFlatInitialization = true;
                // printf("Debug: Detected flat initialization with %zu elements (total: %d)\n",
                // initValues.size(),
                // realTotalElements);
            }
        }

        // 检查是否是混合初始化（包含基础常量和GlobalVariable的混合）
        if (arrayType->getElementType()->isArrayType() && initValues.size() > static_cast<size_t>(expectedElements) &&
            initValues.size() <= static_cast<size_t>(realTotalElements)) {
            bool hasBasicConstants = false;
            bool hasGlobalVars = false;
            for (Value * val: initValues) {
                if (dynamic_cast<ConstInt *>(val) || dynamic_cast<ConstFloat *>(val)) {
                    hasBasicConstants = true;
                } else if (dynamic_cast<GlobalVariable *>(val)) {
                    hasGlobalVars = true;
                }
            }
            if (hasBasicConstants && hasGlobalVars) {
                isMixedInitialization = true;
                // printf("Debug: Detected mixed initialization with %zu elements\n", initValues.size());
            }
        }

        // 对于所有情况，如果初始化值不足，都需要用零填充
        if (initValues.size() < static_cast<size_t>(expectedElements)) {
            printf("Warning: Initializer has %zu elements, but array expects %d elements\n",
                   initValues.size(),
                   expectedElements);

            // 如果初始化值不足，用零填充
            while (initValues.size() < static_cast<size_t>(expectedElements)) {
                if (innerElementType && innerElementType->isFloatType()) {
                    initValues.push_back(module->newConstFloat(0.0f));
                } else {
                    initValues.push_back(module->newConstInt(0));
                }
            }
            // printf("Debug: Filled array with zeros to %zu elements\n", initValues.size());
        }

        // 如果初始化值过多，截断（但不截断扁平化和混合初始化）
        if (!isFlatInitialization && !isMixedInitialization &&
            initValues.size() > static_cast<size_t>(expectedElements)) {
            initValues.resize(expectedElements);
        }
    } else if (node->parent && node->parent->type && node->parent->type->isArrayType()) {
        // 备用方案：检查父节点是否提供了目标数组类型信息
        arrayType = static_cast<ArrayType *>(node->parent->type);
        // printf("Debug: Using parent-provided array type: %s\n", arrayType->toString().c_str());

        // 验证初始化值数量是否匹配
        // 对于多维数组，需要计算所有维度的总元素个数
        int expectedElements = 1;

        // 递归计算所有嵌套维度的总元素个数
        const Type * currentType = arrayType;
        while (currentType && currentType->isArrayType()) {
            const ArrayType * currentArrayType = static_cast<const ArrayType *>(currentType);
            const std::vector<int> & dims = currentArrayType->getDimensions();
            if (!dims.empty()) {
                expectedElements *= dims[0];
            }
            currentType = currentArrayType->getElementType();
        }

        // printf("Debug: Calculated expected elements: %d for array type %s\n",
        // expectedElements,
        // arrayType->toString().c_str());

        if (initValues.size() != static_cast<size_t>(expectedElements)) {
            printf("Warning: Initializer has %zu elements, but array expects %d elements\n",
                   initValues.size(),
                   expectedElements);

            // 如果初始化值不足，用零填充
            while (initValues.size() < static_cast<size_t>(expectedElements)) {
                if (innerElementType && innerElementType->isFloatType()) {
                    initValues.push_back(module->newConstFloat(0.0f));
                } else {
                    initValues.push_back(module->newConstInt(0));
                }
            }
            // 如果初始化值过多，截断
            if (initValues.size() > static_cast<size_t>(expectedElements)) {
                initValues.resize(expectedElements);
            }
        }
    } else if (!initValues.empty() && dynamic_cast<GlobalVariable *>(initValues[0])) {
        // 多维数组：子元素是全局常量数组
        Type * innerType = initValues[0]->getType();
        std::vector<int> dimensions = {static_cast<int>(initValues.size())};
        arrayType = new ArrayType(innerType, dimensions);

        // printf("Debug: Created nested array type [%d x %s]\n", dimensions[0], innerType->toString().c_str());
    } else {
        // 一维数组：子元素是基础类型常量
        // 但是需要检查是否是多维数组的扁平化初始化
        bool isMultiDimFlat = false;

        // 检查是否有嵌套的GlobalVariable（表示嵌套数组）
        for (Value * val: initValues) {
            if (dynamic_cast<GlobalVariable *>(val)) {
                isMultiDimFlat = true;
                break;
            }
        }

        // 如果没有GlobalVariable，检查AST结构来判断是否是多维数组
        if (!isMultiDimFlat) {
            // 检查是否有嵌套的数组初始化节点
            for (auto son: node->sons) {
                if (son->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
                    isMultiDimFlat = true;
                    // printf("Debug: Found nested array init in AST, treating as multi-dim\n");
                    break;
                }
            }
        }

        if (isMultiDimFlat) {
            // 这是一个多维数组的扁平化初始化，尝试推断正确的维度
            // 假设这是一个2D数组，尝试找到合理的行列分布
            int totalElements = static_cast<int>(initValues.size());

            // 分析嵌套数组的结构来推断正确的维度
            int nestedArrayCount = 0;
            int maxNestedSize = 0;

            for (auto son: node->sons) {
                if (son->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
                    nestedArrayCount++;
                    maxNestedSize = std::max(maxNestedSize, static_cast<int>(son->sons.size()));
                }
            }

            // printf("Debug: Found %d nested arrays, max nested size: %d, total elements: %d\n",
            // nestedArrayCount,
            // maxNestedSize,
            // totalElements);

            int bestRows, bestCols;

            if (nestedArrayCount > 0 && maxNestedSize > 0) {
                // 基于嵌套数组的结构推断
                // 假设每个嵌套数组代表一行，最大嵌套大小代表列数
                bestCols = maxNestedSize;

                // 计算需要多少行来容纳所有元素
                // 考虑到有些元素可能不在嵌套数组中（如单独的7）
                int elementsInNestedArrays = 0;
                for (auto son: node->sons) {
                    if (son->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
                        elementsInNestedArrays += son->sons.size();
                    }
                }
                int singleElements = totalElements - elementsInNestedArrays;

                // 总行数 = 嵌套数组数量 + 单独元素需要的行数
                bestRows = nestedArrayCount + (singleElements + bestCols - 1) / bestCols;

                // printf("Debug: Inferred from structure: %d nested arrays + %d single elements → [%d x %d]\n",
                // nestedArrayCount,
                // singleElements,
                // bestRows,
                // bestCols);
            } else {
                // 回退到原来的算法
                bestRows = 1;
                bestCols = totalElements;
                for (int rows = 1; rows * rows <= totalElements; ++rows) {
                    if (totalElements % rows == 0) {
                        int cols = totalElements / rows;
                        if (abs(rows - cols) < abs(bestRows - bestCols)) {
                            bestRows = rows;
                            bestCols = cols;
                        }
                    }
                }
                // printf("Debug: Fallback inference: [%d x %d] from %d elements\n", bestRows, bestCols, totalElements);
            }

            // printf("Debug: Final inferred dimensions: [%d x %d] from %d elements\n", bestRows, bestCols,
            // totalElements);

            // 创建内层数组类型
            std::vector<int> innerDims = {bestCols};
            ArrayType * innerArrayType = new ArrayType(innerElementType, innerDims);

            // 创建外层数组类型
            std::vector<int> outerDims = {bestRows};
            arrayType = new ArrayType(innerArrayType, outerDims);

            // printf("Debug: Created inferred multi-dim array type %s\n", arrayType->toString().c_str());
        } else {
            std::vector<int> dimensions = {static_cast<int>(initValues.size())};
            arrayType = new ArrayType(innerElementType, dimensions);
            // printf("Debug: Created simple array type [%d x %s]\n", dimensions[0],
            // innerElementType->toString().c_str());
        }
    }

    // 检查是否是顶层数组初始化（通过检查父节点类型）
    bool isTopLevel = true;
    if (node->parent && node->parent->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
        isTopLevel = false;
    }

    // 如果有目标数组类型且是顶层，使用目标类型
    if (targetArrayType && isTopLevel) {
        arrayType = targetArrayType;
        // printf("Debug: Using target array type: %s\n", arrayType->toString().c_str());

        // 重新组织初始化值以匹配目标类型
        std::vector<Value *> reorganizedValues;
        if (reorganizeInitValuesForTargetType(initValues, arrayType, reorganizedValues)) {
            initValues = reorganizedValues;
            // printf("Debug: Reorganized init values for target type\n");
        } else {
            // printf("Warning: Failed to reorganize init values, using original values\n");
        }
    }

    if (isTopLevel) {
        // 顶层数组：创建真正的全局常量数组

        // 如果有父节点提供的正确类型，使用processArrayInitialization重新处理
        if (targetArrayType && !node->name.empty()) {
            // printf("Debug: Re-processing with processArrayInitialization for target type: %s\n",
            // targetArrayType->toString().c_str());

            // 使用processArrayInitialization重新处理初始化值
            std::vector<Value *> reorganizedValues;
            if (processArrayInitialization(node, targetArrayType, reorganizedValues)) {
                initValues = reorganizedValues;
                arrayType = targetArrayType;
                // printf("Debug: Successfully re-processed with correct type\n");
            } else {
                printf("Warning: Failed to re-process with target type, using inferred type\n");
            }
        }

        std::string globalArrayName;
        if (!node->name.empty()) {
            // 检查是否是常量数组（通过检查调用上下文）
            // 如果当前没有函数上下文，说明是全局作用域，需要进一步判断
            Function * currentFunc = module->getCurrentFunction();
            if (!currentFunc) {
                // 全局作用域：普通全局数组变量，直接使用变量名
                globalArrayName = node->name;
            } else {
                // 局部作用域：使用__const.main.前缀，并添加唯一计数器避免重名
                static int localArrayCounter = 0;
                globalArrayName = "__const.main." + node->name + "." + std::to_string(localArrayCounter++);
            }
        }
        // printf("Debug: Creating top-level array with name '%s', type %s\n",
        // globalArrayName.c_str(),
        // arrayType->toString().c_str());
        GlobalVariable * constArray = module->newGlobalConstArray(arrayType, globalArrayName);

        // 对于空初始化列表，不填充零值，让GlobalVariable的toString处理
        if (initValues.empty() && arrayType) {
            // printf("Debug: Empty initializer detected, will use zeroinitializer in LLVM IR\n");
            //  不填充零值，保持initValues为空，这样GlobalVariable会输出zeroinitializer
        }

        // 对于2D数组，需要重新组织扁平化的初始化值
        std::vector<Value *> finalInitValues;
        // printf("Debug: Array element type is array: %s\n",
        // arrayType->getElementType()->isArrayType() ? "true" : "false");
        if (arrayType->getElementType()->isArrayType()) {
            // 这是多维数组，直接扁平化所有初始化值
            // printf("Debug: Flattening multi-dimensional array initialization\n");

            // 计算总的基础元素个数
            int totalElements = 1;
            const Type * currentType = arrayType;
            while (currentType && currentType->isArrayType()) {
                const ArrayType * currentArrayType = static_cast<const ArrayType *>(currentType);
                const std::vector<int> & dims = currentArrayType->getDimensions();
                if (!dims.empty()) {
                    totalElements *= dims[0];
                }
                currentType = currentArrayType->getElementType();
            }

            // printf("Debug: Total elements needed: %d, got %zu init values\n", totalElements, initValues.size());

            // 扁平化所有初始化值
            std::vector<Value *> flatValues;

            // 检查是否是扁平初始化（所有元素都是常量）
            bool isFlatInit = true;
            for (auto value: initValues) {
                if (!dynamic_cast<ConstInt *>(value) && !dynamic_cast<ConstFloat *>(value)) {
                    isFlatInit = false;
                    break;
                }
            }

            // printf("Debug: Initialization type: %s\n", isFlatInit ? "flat" : "nested");

            if (isFlatInit) {
                // 扁平初始化：直接按顺序添加所有值（C语言的数组初始化是按行优先顺序）
                // printf("Debug: Flat initialization - adding values in order\n");

                for (auto value: initValues) {
                    flatValues.push_back(value);
                    if (auto constInt = dynamic_cast<ConstInt *>(value)) {
                        // printf("Debug: Added flat value %d at position %zu\n",
                        // constInt->getVal(),
                        // flatValues.size() - 1);
                    }
                }
            } else {
                // 嵌套初始化：按照C语言的初始化规则处理
                // const std::vector<int> & outerDims = arrayType->getDimensions();
                int rowSize = 1;
                if (arrayType->getElementType()->isArrayType()) {
                    const ArrayType * innerArrayType = static_cast<const ArrayType *>(arrayType->getElementType());
                    const std::vector<int> & innerDims = innerArrayType->getDimensions();
                    if (!innerDims.empty()) {
                        rowSize = innerDims[0];
                    }
                }

                // printf("Debug: Array dimensions - outer: %d, inner row size: %d\n",
                // outerDims.empty() ? 0 : outerDims[0],
                // rowSize);

                // 按照C语言的初始化规则，需要重新分析原始的AST节点
                // 而不是使用已经展开的initValues
                int currentPos = 0; // 当前在扁平化数组中的位置

                for (auto son: node->sons) {
                    if (son->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
                        // 嵌套数组初始化，如 {1, 2}
                        // 这应该填充一个完整的行
                        int rowStart = (currentPos / rowSize) * rowSize; // 当前行的起始位置

                        // 确保flatValues有足够的空间到当前行
                        while (flatValues.size() < static_cast<size_t>(rowStart)) {
                            if (arrayType->getElementType()->isFloatType()) {
                                flatValues.push_back(module->newConstFloat(0.0f));
                            } else {
                                flatValues.push_back(module->newConstInt(0));
                            }
                        }

                        // 处理嵌套数组的元素
                        int elementsInRow = 0;
                        for (auto grandson: son->sons) {
                            if (elementsInRow >= rowSize)
                                break; // 不超过行大小

                            if (grandson->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                                flatValues.push_back(module->newConstInt(grandson->integer_val));
                                // printf("Debug: Added nested element %d at position %zu\n",
                                // grandson->integer_val,
                                // flatValues.size() - 1);
                            } else if (grandson->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
                                flatValues.push_back(module->newConstFloat(grandson->float_val));
                            } else {
                                if (arrayType->getElementType()->isFloatType()) {
                                    flatValues.push_back(module->newConstFloat(0.0f));
                                } else {
                                    flatValues.push_back(module->newConstInt(0));
                                }
                            }
                            elementsInRow++;
                        }

                        // 用零填充当前行的剩余位置
                        while (elementsInRow < rowSize) {
                            if (arrayType->getElementType()->isFloatType()) {
                                flatValues.push_back(module->newConstFloat(0.0f));
                            } else {
                                flatValues.push_back(module->newConstInt(0));
                            }
                            elementsInRow++;
                        }

                        currentPos = flatValues.size(); // 移动到下一行
                        // printf("Debug: Completed nested array row, currentPos = %d\n", currentPos);

                    } else if (son->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT ||
                               son->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
                        // 单个值，按顺序填充
                        if (son->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                            flatValues.push_back(module->newConstInt(son->integer_val));
                            // printf("Debug: Added single element %d at position %zu\n",
                            // son->integer_val,
                            // flatValues.size() - 1);
                        } else {
                            flatValues.push_back(module->newConstFloat(son->float_val));
                        }
                        currentPos++;
                    }
                }
            }

            // 用零填充不足的元素，但不超过总元素数
            while (flatValues.size() < static_cast<size_t>(totalElements)) {
                if (arrayType->getElementType()->isFloatType()) {
                    flatValues.push_back(module->newConstFloat(0.0f));
                } else {
                    flatValues.push_back(module->newConstInt(0));
                }
            }

            // 确保不超过总元素数
            if (flatValues.size() > static_cast<size_t>(totalElements)) {
                flatValues.resize(totalElements);
                // printf("Debug: Trimmed flat values to %d elements\n", totalElements);
            }

            // printf("Debug: Flattened to %zu values, using as final init values\n", flatValues.size());
            finalInitValues = flatValues;
        } else {
            finalInitValues = initValues;
        }

        constArray->setInitValueList(finalInitValues);
        node->val = constArray;
        // printf("Debug: Created top-level global array: %s with type %s\n",
        // constArray->getIRName().c_str(),
        // arrayType->toString().c_str());
    } else {
        // 嵌套数组：对于常量数组初始化，直接扁平化而不创建临时数组
        // 检查是否是用于常量数组的嵌套初始化（通过检查节点名称是否为空来判断）
        bool isNestedConstantInit = (node->name.empty() || node->name.find("__const.") != std::string::npos);

        if (isNestedConstantInit) {
            // 这是多维数组，直接扁平化所有初始化值
            // printf("Debug: Creating flattened inline array (no temp array)\n");

            // 直接将扁平化的值作为节点值，不创建全局变量
            std::vector<Value *> flatValues;
            for (auto value: initValues) {
                if (auto constInt = dynamic_cast<ConstInt *>(value)) {
                    flatValues.push_back(constInt);
                } else if (auto constFloat = dynamic_cast<ConstFloat *>(value)) {
                    flatValues.push_back(constFloat);
                }
            }

            // 计算总的基础元素个数
            int totalElements = 1;
            const Type * currentType = arrayType;
            while (currentType && currentType->isArrayType()) {
                const ArrayType * currentArrayType = static_cast<const ArrayType *>(currentType);
                const std::vector<int> & dims = currentArrayType->getDimensions();
                if (!dims.empty()) {
                    totalElements *= dims[0];
                }
                currentType = currentArrayType->getElementType();
            }

            // 用零填充不足的元素
            while (flatValues.size() < static_cast<size_t>(totalElements)) {
                if (arrayType->getElementType()->isFloatType()) {
                    flatValues.push_back(module->newConstFloat(0.0f));
                } else {
                    flatValues.push_back(module->newConstInt(0));
                }
            }

            // 创建一个临时的全局变量来存储嵌套数组的值
            // 但是我们需要确保父数组能够正确展开这些值
            static int nestedArrayCounter = 0;
            std::string nestedArrayName = "__nested_array_" + std::to_string(nestedArrayCounter++);

            GlobalVariable * nestedArray = module->newGlobalConstArray(arrayType, nestedArrayName);
            nestedArray->setInitValueList(flatValues);
            nestedArray->setBSSSection(false); // 有初始值，不在BSS段
            nestedArray->setConstant(true);

            node->val = nestedArray;
            // printf("Debug: Created nested array '%s' with %zu elements\n", nestedArrayName.c_str(),
            // flatValues.size());
        } else {
            // 普通数组：创建临时的全局变量
            static int tempArrayCounter = 0;
            std::string tempArrayName = "__temp_array_" + std::to_string(tempArrayCounter++);

            GlobalVariable * tempArray = module->newGlobalConstArray(arrayType, tempArrayName);
            tempArray->setInitValueList(initValues);
            tempArray->setBSSSection(false); // 有初始值，不在BSS段
            node->val = tempArray;
            // printf("Debug: Created temporary nested array for inlining with %zu elements\n", initValues.size());
        }
    }

    return true;
}

/// @brief 递归处理数组初始化，用于全局常量数组
/// @param initNode 数组初始化节点
/// @param arrayType 数组类型
/// @param initValues 输出的初始化值列表
/// @return 是否成功
bool IRGenerator::processArrayInitialization(ast_node * initNode,
                                             ArrayType * arrayType,
                                             std::vector<Value *> & initValues)
{
    if (!initNode || initNode->node_type != ast_operator_type::AST_OP_ARRAY_INIT) {
        printf("Error: Invalid array initialization node.\n");
        return false;
    }

    const std::vector<int> & dimensions = arrayType->getDimensions();
    if (dimensions.empty()) {
        printf("Error: Array type has no dimensions.\n");
        return false;
    }

    // 检查是否是多维数组
    bool isMultiDim = arrayType->getElementType()->isArrayType();

    if (isMultiDim) {
        // 多维数组处理
        ArrayType * innerArrayType = static_cast<ArrayType *>(arrayType->getElementType());
        const std::vector<int> & innerDims = innerArrayType->getDimensions();
        int innerSize = innerDims.empty() ? 1 : innerDims[0]; // 内层数组的大小
        int outerSize = dimensions[0];                        // 外层数组的大小

        // 计算真正的总元素个数（递归计算所有嵌套维度）
        int totalElements = outerSize * innerArrayType->getTotalElements();

        // printf("Debug: Processing multi-dim array [%d][%d], total elements: %d\n", outerSize, innerSize,
        // totalElements);

        // 初始化结果数组，全部填零
        Value * zeroValue = nullptr;
        if (arrayType->getElementType()->isFloatType()) {
            zeroValue = module->newConstFloat(0.0f);
        } else {
            zeroValue = module->newConstInt(0);
        }
        initValues.resize(totalElements, zeroValue);

        int currentPos = 0; // 当前在扁平化数组中的位置

        for (auto son: initNode->sons) {
            if (son->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                // 单个整数字面量 - 需要考虑是否应该开始新行
                if (currentPos < totalElements) {
                    // 如果当前位置不在行首且前面有嵌套数组，移动到下一行
                    if (currentPos % innerSize != 0) {
                        int nextRowStart = ((currentPos / innerSize) + 1) * innerSize;
                        if (nextRowStart < totalElements) {
                            currentPos = nextRowStart;
                        }
                    }

                    if (currentPos < totalElements) {
                        initValues[currentPos] = module->newConstInt(son->integer_val);
                        currentPos++;
                    }
                }
            } else if (son->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
                // 单个浮点数字面量 - 需要考虑是否应该开始新行
                if (currentPos < totalElements) {
                    // 如果当前位置不在行首且前面有嵌套数组，移动到下一行
                    if (currentPos % innerSize != 0) {
                        int nextRowStart = ((currentPos / innerSize) + 1) * innerSize;
                        if (nextRowStart < totalElements) {
                            currentPos = nextRowStart;
                        }
                    }

                    if (currentPos < totalElements) {
                        initValues[currentPos] = module->newConstFloat(son->float_val);
                        currentPos++;
                    }
                }
            } else if (son->node_type == ast_operator_type::AST_OP_NEGATIVE) {
                // 处理负数常量
                Value * constResult = nullptr;
                if (evaluate_const_expr(son, constResult)) {
                    if (currentPos < totalElements) {
                        // 如果当前位置不在行首且前面有嵌套数组，移动到下一行
                        if (currentPos % innerSize != 0) {
                            int nextRowStart = ((currentPos / innerSize) + 1) * innerSize;
                            if (nextRowStart < totalElements) {
                                currentPos = nextRowStart;
                            }
                        }

                        if (currentPos < totalElements) {
                            initValues[currentPos] = constResult;
                            currentPos++;
                        }
                    }
                } else {
                    printf("Error: Failed to evaluate negative constant in array initialization\n");
                    return false;
                }
            } else if (son->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
                // 嵌套数组初始化 - 填充一整行
                int rowStart = (currentPos / innerSize) * innerSize; // 当前行的起始位置
                if (currentPos % innerSize != 0) {
                    // 如果当前位置不在行首，移动到下一行
                    rowStart += innerSize;
                }

                std::vector<Value *> nestedValues;
                if (!processArrayInitialization(son, innerArrayType, nestedValues)) {
                    return false;
                }

                // 将嵌套数组的值复制到对应的行
                for (size_t i = 0; i < nestedValues.size() && rowStart + i < static_cast<size_t>(totalElements); ++i) {
                    initValues[rowStart + i] = nestedValues[i];
                }

                currentPos = rowStart + innerSize; // 移动到下一行
            }
        }
    } else {
        // 一维数组处理
        int totalElements = arrayType->getTotalElements();
        // printf("Debug: Processing 1D array, total elements: %d\n", totalElements);

        for (auto son: initNode->sons) {
            if (son->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                initValues.push_back(module->newConstInt(son->integer_val));
            } else if (son->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
                initValues.push_back(module->newConstFloat(son->float_val));
            } else if (son->node_type == ast_operator_type::AST_OP_NEGATIVE) {
                // 处理负数常量
                Value * constResult = nullptr;
                if (evaluate_const_expr(son, constResult)) {
                    initValues.push_back(constResult);
                } else {
                    printf("Error: Failed to evaluate negative constant in array initialization\n");
                    return false;
                }
            }
        }

        // 对于大数组，避免生成过多的零值
        if (initValues.empty()) {
            // 完全空的初始化列表，不填充零值，使用zeroinitializer
            // printf("Debug: Empty initialization list, will use zeroinitializer\n");
        } else if (static_cast<int>(initValues.size()) < totalElements) {
            // 部分初始化，只有在数组不太大时才填充零值
            if (totalElements <= 10000) { // 限制填充的最大元素数
                while (static_cast<int>(initValues.size()) < totalElements) {
                    if (arrayType->getElementType()->isIntegerType()) {
                        initValues.push_back(module->newConstInt(0));
                    } else if (arrayType->getElementType()->isFloatType()) {
                        initValues.push_back(module->newConstFloat(0.0f));
                    } else {
                        initValues.push_back(module->newConstInt(0));
                    }
                }
            } else {
                printf("Warning: Array too large (%d elements), partial initialization not supported\n", totalElements);
            }
        }
    }

    // printf("Debug: processArrayInitialization completed with %zu values\n", initValues.size());
    return true;
}

/// @brief 重新组织初始化值以匹配目标数组类型
/// @param flatValues 扁平化的初始化值
/// @param targetType 目标数组类型
/// @param reorganizedValues 输出的重新组织后的值
/// @return 是否成功
bool IRGenerator::reorganizeInitValuesForTargetType(const std::vector<Value *> & flatValues,
                                                    ArrayType * targetType,
                                                    std::vector<Value *> & reorganizedValues)
{
    if (!targetType || !targetType->isArrayType()) {
        return false;
    }

    const std::vector<int> & dimensions = targetType->getDimensions();
    if (dimensions.empty()) {
        return false;
    }

    // 对于二维数组，重新组织扁平化的值
    if (dimensions.size() == 2) {
        int rows = dimensions[0];
        int cols = dimensions[1];
        int totalElements = rows * cols;

        // printf("Debug: Reorganizing for 2D array [%d x %d], total elements: %d\n", rows, cols, totalElements);

        // 确保有足够的值
        if (static_cast<int>(flatValues.size()) > totalElements) {
            printf("Warning: Too many init values (%zu) for target array (%d elements)\n",
                   flatValues.size(),
                   totalElements);
            return false;
        }

        // 直接使用扁平化的值，LLVM会正确处理多维数组的内存布局
        reorganizedValues = flatValues;

        // 如果值不足，用零填充
        while (static_cast<int>(reorganizedValues.size()) < totalElements) {
            if (targetType->getElementType()->isFloatType()) {
                reorganizedValues.push_back(module->newConstFloat(0.0f));
            } else {
                reorganizedValues.push_back(module->newConstInt(0));
            }
        }

        return true;
    }

    // 对于其他维度，暂时不支持
    return false;
}

/// @brief 检测数组初始化是否包含动态值
/// @param initNode 数组初始化节点
/// @return true：包含动态值，false：纯静态值
bool IRGenerator::hasRuntimeValues(ast_node * initNode)
{
    if (!initNode) {
        return false;
    }

    // printf("Debug: hasRuntimeValues checking node type %d with %zu children\n",
    //(int) initNode->node_type,
    // initNode->sons.size());

    // 递归检查所有子节点
    for (ast_node * child: initNode->sons) {
        // printf("Debug: hasRuntimeValues checking child node type %d\n", (int) child->node_type);

        switch (child->node_type) {
            case ast_operator_type::AST_OP_LEAF_LITERAL_UINT:
            case ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT:
                // 字面量常量，静态值
                // printf("Debug: Found literal constant, treating as static\n");
                continue;

            case ast_operator_type::AST_OP_ARRAY_INIT:
                // 嵌套数组初始化，递归检查
                // printf("Debug: Found nested array init, recursively checking\n");
                if (hasRuntimeValues(child)) {
                    // printf("Debug: Nested array init contains runtime values\n");
                    return true;
                } else {
                    // printf("Debug: Nested array init is static\n");
                }
                continue;

            case ast_operator_type::AST_OP_ARRAY_ACCESS:
                // 数组访问，需要检查是否可以在编译时求值
                {
                    // printf("Debug: Found array access, checking if compile-time evaluable\n");
                    Value * constResult = nullptr;
                    if (evaluate_const_expr(child, constResult)) {
                        // 可以在编译时求值，视为静态值
                        // printf("Debug: Array access can be evaluated at compile time, treating as static\n");
                        continue;
                    } else {
                        // 无法在编译时求值，视为动态值
                        // printf("Debug: Found runtime value: array access\n");
                        return true;
                    }
                }

            case ast_operator_type::AST_OP_LEAF_VAR_ID:
                // 变量引用，动态值
                // printf("Debug: Found runtime value: variable\n");
                return true;

            case ast_operator_type::AST_OP_ADD:
            case ast_operator_type::AST_OP_SUB:
            case ast_operator_type::AST_OP_MUL:
            case ast_operator_type::AST_OP_DIV:
            case ast_operator_type::AST_OP_MOD:
                // 算术表达式，需要递归检查操作数
                // printf("Debug: Found arithmetic expression, recursively checking\n");
                if (hasRuntimeValues(child)) {
                    // printf("Debug: Arithmetic expression contains runtime values\n");
                    return true;
                } else {
                    // printf("Debug: Arithmetic expression is static\n");
                }
                continue;

            default:
                // 其他类型的节点，保守地认为是动态值
                // printf("Debug: Unknown node type %d, treating as runtime value\n", (int) child->node_type);
                return true;
        }
    }

    // printf("Debug: hasRuntimeValues returning false (all static)\n");
    return false;
}

/// @brief 处理零初始化数组
/// @param node AST节点
/// @param arrayVar 数组变量
/// @param arrayType 数组类型
/// @return 翻译是否成功
bool IRGenerator::handleZeroInitialization(ast_node * node, Value * arrayVar, ArrayType * arrayType)
{
    Function * currentFunc = module->getCurrentFunction();

    // 1. 将数组转换为 i8*
    BitcastInstruction * destCast = new BitcastInstruction(currentFunc, arrayVar, module->getI8PtrType());
    node->blockInsts.addInst(destCast);

    // 2. 计算数组大小
    int totalSize = arrayType->getSize();
    ConstInt * sizeConst = module->newConstInt(totalSize);

    // 3. 生成 memset 指令清零
    MemsetInstruction * memsetInst =
        new MemsetInstruction(currentFunc, destCast, module->newConstInt(0), sizeConst, false);
    node->blockInsts.addInst(memsetInst);

    // printf("Debug: Generated memset for zero initialization, size = %d\n", totalSize);
    return true;
}

/// @brief 处理静态初始化数组
/// @param node AST节点
/// @param arrayVar 数组变量
/// @param arrayType 数组类型
/// @param initExprNode 初始化表达式节点
/// @param varName 变量名
/// @return 翻译是否成功
bool IRGenerator::handleStaticInitialization(ast_node * node,
                                             Value * arrayVar,
                                             ArrayType * arrayType,
                                             ast_node * initExprNode,
                                             const std::string & varName)
{
    Function * currentFunc = module->getCurrentFunction();

    // 1. 处理初始化表达式，生成全局常量数组
    initExprNode->type = arrayType;
    // 将变量名临时存储在AST节点中，供ir_array_init使用
    initExprNode->name = varName;
    if (!ir_visit_ast_node(initExprNode)) {
        printf("Error: Failed to process static initialization expression\n");
        return false;
    }
    node->blockInsts.addInst(initExprNode->blockInsts);

    GlobalVariable * constArray = static_cast<GlobalVariable *>(initExprNode->val);
    if (!constArray) {
        printf("Error: Failed to get constant array from static initialization\n");
        return false;
    }

    // 2. 全局数组的名称已经在创建时设置为正确的格式

    // 3. 将局部数组转换为 i8*
    BitcastInstruction * destCast = new BitcastInstruction(currentFunc, arrayVar, module->getI8PtrType());
    node->blockInsts.addInst(destCast);

    // 4. 将常量数组转换为 i8*
    BitcastInstruction * srcCast = new BitcastInstruction(currentFunc, constArray, module->getI8PtrType());
    node->blockInsts.addInst(srcCast);

    // 5. 计算拷贝大小
    int totalSize = arrayType->getSize();
    ConstInt * sizeConst = module->newConstInt(totalSize);

    // 6. 生成 memcpy 指令
    MemcpyInstruction * memcpyInst = new MemcpyInstruction(currentFunc, destCast, srcCast, sizeConst, false);
    node->blockInsts.addInst(memcpyInst);

    // printf("Debug: Generated memcpy for static initialization, size = %d\n", totalSize);
    return true;
}

/// @brief 处理动态初始化数组
/// @param node AST节点
/// @param arrayVar 数组变量
/// @param arrayType 数组类型
/// @param initExprNode 初始化表达式节点
/// @return 翻译是否成功
bool IRGenerator::handleDynamicInitialization(ast_node * node,
                                              Value * arrayVar,
                                              ArrayType * arrayType,
                                              ast_node * initExprNode)
{
    Function * currentFunc = module->getCurrentFunction();

    // printf("Debug: Processing dynamic initialization with %zu elements\n", initExprNode->sons.size());

    // 获取数组的维度信息
    const std::vector<int> & outerDimensions = arrayType->getDimensions();

    // 检查内层类型，判断是一维数组还是多维数组
    Type * innerType = arrayType->getElementType();

    // 如果是一维数组（内层类型不是数组类型），使用简单的逐元素赋值
    if (outerDimensions.size() == 1 && !innerType->isArrayType()) {
        // printf("Debug: Processing 1D array dynamic initialization\n");
        return handleOneDimensionalDynamicInit(node, arrayVar, arrayType, initExprNode);
    }

    // 以下是原有的多维数组处理逻辑
    if (outerDimensions.size() != 1) {
        printf("Error: Expected outer array dimension size 1, got %zu\n", outerDimensions.size());
        return false;
    }

    // 检查是否是动态数组（通过检查当前维度来推断）
    bool isDynamicArray = false;

    // 如果外层维度是-1，说明这是动态数组
    if (outerDimensions[0] == -1) {
        isDynamicArray = true;
        // printf("Debug: Confirmed this is a dynamic array (dimension = -1)\n");
    } else {
        // printf("Debug: Array has fixed dimension: %d\n", outerDimensions[0]);
    }

    // 检查内层是否也是数组类型（多维数组情况）
    if (!innerType->isArrayType()) {
        printf("Error: Expected inner array type for 2D array\n");
        return false;
    }

    const ArrayType * innerArrayType = static_cast<const ArrayType *>(innerType);
    const std::vector<int> & innerDimensions = innerArrayType->getDimensions();
    if (innerDimensions.size() != 1) {
        printf("Error: Expected inner array dimension size 1, got %zu\n", innerDimensions.size());
        return false;
    }

    int cols = innerDimensions[0];

    // 处理混合的初始化列表（包含嵌套数组和单个元素）
    // 我们需要智能地将初始化元素分组到行中

    std::vector<std::vector<ast_node *>> rowElements;
    std::vector<ast_node *> currentRowElements;

    for (size_t i = 0; i < initExprNode->sons.size(); ++i) {
        ast_node * initElement = initExprNode->sons[i];

        if (initElement->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
            // 嵌套数组初始化，这表示一个完整的行
            if (!currentRowElements.empty()) {
                // 如果当前行有元素，先完成当前行
                rowElements.push_back(currentRowElements);
                currentRowElements.clear();
            }

            // 将嵌套数组的元素作为一行
            std::vector<ast_node *> nestedRow;
            for (size_t j = 0; j < initElement->sons.size(); ++j) {
                ast_node * element = initElement->sons[j];

                // 特殊处理：如果元素本身也是一个只包含单个运行时值的嵌套数组初始化
                // 例如 {c[2][1]}，直接提取其中的运行时值，不要创建嵌套数组
                if (element->node_type == ast_operator_type::AST_OP_ARRAY_INIT && element->sons.size() == 1 &&
                    element->sons[0]->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
                    // 直接使用其中的数组访问元素，跳过嵌套数组包装
                    nestedRow.push_back(element->sons[0]);
                } else {
                    nestedRow.push_back(element);
                }
            }
            rowElements.push_back(nestedRow);

            // printf("Debug: Added nested array as row %zu with %zu elements\n",
            // rowElements.size() - 1,
            // nestedRow.size());
        } else {
            // 单个元素，添加到当前行
            currentRowElements.push_back(initElement);

            // 如果当前行已满，完成这一行
            if (static_cast<int>(currentRowElements.size()) >= cols) {
                rowElements.push_back(currentRowElements);
                currentRowElements.clear();
                // printf("Debug: Completed row %zu with %d elements\n", rowElements.size() - 1, cols);
            }
        }
    }

    // 如果还有未完成的行，添加它
    if (!currentRowElements.empty()) {
        rowElements.push_back(currentRowElements);
        // printf("Debug: Added final row %zu with %zu elements\n", rowElements.size() - 1, currentRowElements.size());
    }

    // printf("Debug: Organized %zu elements into %zu rows\n", initExprNode->sons.size(), rowElements.size());

    // 对于动态数组，使用组织后的行数
    int rows = (outerDimensions[0] == -1) ? static_cast<int>(rowElements.size()) : outerDimensions[0];
    // printf("Debug: Array dimensions: [%d x %d] (dynamic rows: %s)\n",
    // rows,
    // cols,
    //(outerDimensions[0] == -1) ? "yes" : "no");

    // 如果是动态数组，需要记录实际大小以便栈分配时使用
    if (isDynamicArray) {
        // 创建新的数组类型，使用实际的行数
        std::vector<int> actualDimensions = {rows};
        ArrayType * actualArrayType = new ArrayType(innerType, actualDimensions);

        // 将实际的数组大小信息存储到变量中，供栈分配时使用
        std::string originalName = arrayVar->getName();

        // printf("Debug: Dynamic array %s actual size: %d bytes (rows: %d)\n",
        // originalName.c_str(),
        // actualArrayType->getSize(),
        // rows);

        // 创建一个新的变量值，使用正确的类型
        std::string sizeInfo = "_ACTUAL_SIZE_" + std::to_string(actualArrayType->getSize());
        std::string newVarName = originalName + sizeInfo;

        // 创建新的变量值，使用正确的类型
        Value * newArrayVar = module->newVarValue(actualArrayType, newVarName);
        if (!newArrayVar) {
            printf("Error: Failed to create new array variable with correct type\n");
            return false;
        }

        // 现在创建正确的alloca指令，使用实际的数组类型和新变量
        AllocaInstruction * allocaInst = new AllocaInstruction(currentFunc, newArrayVar, actualArrayType, 16);
        node->blockInsts.addInst(allocaInst);
        // printf("Debug: Created alloca for dynamic array with actual type: %s\n",
        // actualArrayType->toString().c_str());

        // 更新arrayVar指向新的变量，这样后续的getelementptr会使用正确的类型
        arrayVar = newArrayVar;
        // printf("Debug: Updated arrayVar to use actual type: %s\n", actualArrayType->toString().c_str());

        // 同时更新arrayType，确保后续的处理使用正确的类型
        arrayType = actualArrayType;
        // printf("Debug: Updated arrayType to: %s\n", arrayType->toString().c_str());
    }

    // 按行列顺序填充数组
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // 生成目标地址：arrayVar[row][col]
            ConstInt * rowIndex = module->newConstInt(row);
            ConstInt * colIndex = module->newConstInt(col);
            ConstInt * zeroConst = module->newConstInt(0);

            // 首先获取行地址：arrayVar[0][row]
            GetelementptrInstruction * rowGepInst =
                new GetelementptrInstruction(currentFunc, arrayVar, zeroConst, rowIndex);
            node->blockInsts.addInst(rowGepInst);

            // 然后获取列地址：row[0][col]
            GetelementptrInstruction * colGepInst =
                new GetelementptrInstruction(currentFunc, rowGepInst, zeroConst, colIndex);
            node->blockInsts.addInst(colGepInst);

            Value * elementValue = nullptr;

            if (row < static_cast<int>(rowElements.size()) && col < static_cast<int>(rowElements[row].size())) {
                // 有显式初始化值
                ast_node * elementNode = rowElements[row][col];

                // 处理元素值
                if (!ir_visit_ast_node(elementNode)) {
                    printf("Error: Failed to process element [%d][%d]\n", row, col);
                    return false;
                }
                node->blockInsts.addInst(elementNode->blockInsts);

                elementValue = elementNode->val;

                // 如果是数组访问或变量，需要load
                if (needsLoad(elementValue)) {
                    LoadInstruction * loadInst = new LoadInstruction(currentFunc, elementValue, elementValue, 4);
                    node->blockInsts.addInst(loadInst);
                    elementValue = loadInst;
                }

                // printf("Debug: Stored element [%d][%d] with explicit value\n", row, col);
            } else {
                // 没有显式初始化值，使用0
                elementValue = module->newConstInt(0);
                // printf("Debug: Stored element [%d][%d] with default value 0\n", row, col);
            }

            // 存储到目标位置
            // 检查colGepInst的类型，如果仍然是数组类型，需要进一步访问
            Value * finalAddress = colGepInst;
            Type * targetType = colGepInst->getType();

            if (targetType->isPointerType()) {
                const PointerType * ptrType = static_cast<const PointerType *>(targetType);
                const Type * pointeeType = ptrType->getPointeeType();

                // 如果指向的是数组类型，需要进一步访问到元素
                if (pointeeType->isArrayType()) {
                    const ArrayType * arrayType = static_cast<const ArrayType *>(pointeeType);
                    const std::vector<int> & dimensions = arrayType->getDimensions();

                    // 对于每个剩余的维度，添加[0]索引
                    Value * currentAddress = finalAddress;
                    for (size_t i = 0; i < dimensions.size(); ++i) {
                        ConstInt * zeroIndex = module->newConstInt(0);
                        if (i == 0) {
                            // 第一次访问需要两个索引：[0][0]
                            GetelementptrInstruction * deeperGepInst =
                                new GetelementptrInstruction(currentFunc, currentAddress, zeroIndex, zeroIndex);
                            node->blockInsts.addInst(deeperGepInst);
                            currentAddress = deeperGepInst;
                        } else {
                            // 后续访问只需要一个索引：[0]
                            GetelementptrInstruction * deeperGepInst =
                                new GetelementptrInstruction(currentFunc, currentAddress, zeroIndex);
                            node->blockInsts.addInst(deeperGepInst);
                            currentAddress = deeperGepInst;
                        }
                    }
                    finalAddress = currentAddress;
                }
            }

            // 检查elementValue和finalAddress的类型兼容性
            Value * valueToStore = elementValue;

            // 如果elementValue是数组类型，而finalAddress指向单个元素，需要提取数组的第一个元素
            if (elementValue->getType()->isArrayType() && finalAddress->getType()->isPointerType()) {
                const PointerType * ptrType = static_cast<const PointerType *>(finalAddress->getType());
                const Type * pointeeType = ptrType->getPointeeType();

                // 如果目标是单个元素（不是数组），需要提取数组的第一个元素
                if (!pointeeType->isArrayType()) {

                    // 创建一个临时变量来存储数组值
                    static int tempArrayCounter = 0;
                    std::string tempArrayName = "__temp_array_" + std::to_string(tempArrayCounter++);

                    // 临时数组变量的类型应该是指向数组类型的指针类型
                    Type * arrayType = elementValue->getType();      // 数组类型（如[1 x i32]）
                    Type * tempVarType = new PointerType(arrayType); // 变量类型（如[1 x i32]*）
                    Value * tempVar = module->newVarValue(tempVarType, tempArrayName);
                    if (!tempVar) {
                        printf("Error: Failed to create temporary array variable\n");
                        return false;
                    }
                    AllocaInstruction * tempArrayVar = new AllocaInstruction(currentFunc, tempVar, arrayType, 4);
                    node->blockInsts.addInst(tempArrayVar);

                    // 使用tempVar作为指针，而不是tempArrayVar
                    Value * tempPointer = tempVar;

                    // 存储数组值到临时变量
                    StoreInstruction * tempStoreInst = new StoreInstruction(currentFunc, elementValue, tempPointer, 4);
                    node->blockInsts.addInst(tempStoreInst);

                    // 提取第一个元素
                    ConstInt * zeroIndex = module->newConstInt(0);
                    GetelementptrInstruction * extractGepInst =
                        new GetelementptrInstruction(currentFunc, tempPointer, zeroIndex, zeroIndex);
                    node->blockInsts.addInst(extractGepInst);

                    // 加载第一个元素的值
                    LoadInstruction * loadInst = new LoadInstruction(currentFunc, extractGepInst, extractGepInst, 4);
                    node->blockInsts.addInst(loadInst);

                    valueToStore = loadInst;
                }
            }

            StoreInstruction * storeInst = new StoreInstruction(currentFunc, valueToStore, finalAddress, 4);
            node->blockInsts.addInst(storeInst);
        }
    }

    // printf("Debug: Dynamic initialization completed\n");
    return true;
}

/// @brief 数组变量声明和初始化节点翻译成线性中间IR
/// @param node AST节点
/// @param typeNode 类型节点
/// @param varNode 变量名节点
/// @param initExprNode 数组初始化表达式节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_variable_declare_with_init(ast_node * node,
                                                      ast_node * typeNode,
                                                      ast_node * varNode,
                                                      ast_node * initExprNode)
{
    Function * currentFunc = module->getCurrentFunction();
    ArrayType * arrayType = nullptr;

    // 确定数组类型
    if (typeNode->type->isArrayType()) {
        arrayType = static_cast<ArrayType *>(typeNode->type);
        // printf("Debug: Using existing array type: %s\n", arrayType->toString().c_str());
    } else {
        printf("Error: Expected array type but got %s\n", typeNode->type->toString().c_str());
        return false;
    }

    // 创建局部数组变量
    Value * arrayVar = module->newVarValue(arrayType, varNode->name);
    if (!arrayVar) {
        printf("Error: Failed to create array variable.\n");
        return false;
    }

    // 检查是否是动态数组（包含-1维度）
    bool isDynamicArray = false;
    ArrayType * originalArrayType = arrayType;

    // 检查数组维度中是否有-1
    const std::vector<int> & dimensions = arrayType->getDimensions();
    for (int dim: dimensions) {
        if (dim == -1) {
            isDynamicArray = true;
            break;
        }
    }

    if (isDynamicArray) {
        // printf("Debug: Detected dynamic array %s, will create alloca after determining actual size\n",
        // varNode->name.c_str());
        // 对于动态数组，先不创建alloca，等到处理初始化时再创建
        // 但是我们需要保存原始的动态数组类型信息
        varNode->type = originalArrayType; // 保存原始类型信息
    } else {
        // 创建 alloca 指令，为数组分配栈空间（16字节对齐）
        AllocaInstruction * allocaInst = new AllocaInstruction(currentFunc, arrayVar, arrayType, 16);
        node->blockInsts.addInst(allocaInst);
    }

    // 处理初始化
    if (initExprNode) {
        // 检查初始化类型
        if (initExprNode->node_type == ast_operator_type::AST_OP_ARRAY_INIT && initExprNode->sons.empty()) {
            // 1. 零初始化：空初始化列表 {}
            // printf("Debug: Processing zero initialization for array %s\n", varNode->name.c_str());
            if (!handleZeroInitialization(node, arrayVar, arrayType)) {
                return false;
            }
        } else if (hasRuntimeValues(initExprNode)) {
            // 2. 动态初始化：包含运行时值
            // printf("Debug: Processing dynamic initialization for array %s\n", varNode->name.c_str());
            if (!handleDynamicInitialization(node, arrayVar, arrayType, initExprNode)) {
                return false;
            }

            // 检查是否是动态数组，如果是则需要更新varNode的val引用
            if (arrayType->getDimensions()[0] == -1 || arrayVar->getName().find("_ACTUAL_SIZE_") != std::string::npos) {
                // 动态数组的变量可能已经被更新，需要查找新的变量
                std::string varName = arrayVar->getName();
                if (varName.find("_ACTUAL_SIZE_") != std::string::npos) {
                    // 从名称中提取原始名称
                    std::string originalName = varName.substr(0, varName.find("_ACTUAL_SIZE_"));
                    // printf("Debug: Dynamic array %s was updated, updating varNode reference\n",
                    // originalName.c_str());
                    varNode->val = arrayVar; // 更新varNode的val引用

                    // 同时更新符号表中的变量引用
                    if (module->getCurrentFunction()) {
                        // 在当前函数的符号表中更新变量引用
                        // printf("Debug: Updating symbol table for variable %s\n", originalName.c_str());

                        // 我们需要手动更新符号表中的变量引用
                        // 采用一个变通的方法：创建一个新的变量，使用原始名称，然后插入符号表
                        // 这样后续的查找就能找到更新后的变量

                        // 创建一个新的变量值，使用原始名称和更新后的类型
                        Value * newVar = module->newVarValue(arrayVar->getType(), originalName);
                        if (newVar) {
                            // 将新变量的内部状态设置为与更新后的arrayVar相同
                            newVar->setIRName(arrayVar->getIRName());
                            // printf("Debug: Created new variable in symbol table: %s -> %s\n",
                            // originalName.c_str(),
                            // newVar->getIRName().c_str());
                        } else {
                            // printf("Debug: Variable %s already exists in symbol table, this is expected\n",
                            // originalName.c_str());
                        }
                    }
                }
            }
        } else {
            // 3. 静态初始化：纯常量值
            // printf("Debug: Processing static initialization for array %s\n", varNode->name.c_str());
            if (!handleStaticInitialization(node, arrayVar, arrayType, initExprNode, varNode->name)) {
                return false;
            }
        }
    } else {
        // 没有初始化表达式，使用零初始化
        // printf("Debug: No initializer, using zero initialization for array %s\n", varNode->name.c_str());
        if (!handleZeroInitialization(node, arrayVar, arrayType)) {
            return false;
        }
    }

    // 设置节点的值
    varNode->val = arrayVar;
    node->val = arrayVar;

    return true;
}

/// @brief 整数减法AST节点翻译成线性中间IR（使用已处理的操作数）
bool IRGenerator::ir_sub_processed(ast_node * node, ast_node * left, ast_node * right)
{
    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 处理类型转换
    if (!handleArithmeticTypeConversion(node, leftValue, rightValue)) {
        return false;
    }

    // 生成减法指令
    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(subInst);
    node->val = subInst;

    return true;
}

/// @brief 浮点数减法AST节点翻译成线性中间IR（使用已处理的操作数）
bool IRGenerator::ir_fsub_processed(ast_node * node, ast_node * left, ast_node * right)
{
    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 确保操作数是浮点类型
    if (!leftValue->getType()->isFloatType()) {
        leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
        if (!leftValue)
            return false;
    }
    if (!rightValue->getType()->isFloatType()) {
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue)
            return false;
    }

    // 生成浮点减法指令
    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_F,
                                                        leftValue,
                                                        rightValue,
                                                        FloatType::getTypeFloat());
    node->blockInsts.addInst(subInst);
    node->val = subInst;

    return true;
}

/// @brief 整数乘法AST节点翻译成线性中间IR（使用已处理的操作数）
bool IRGenerator::ir_mul_processed(ast_node * node, ast_node * left, ast_node * right)
{
    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 处理类型转换
    if (!handleArithmeticTypeConversion(node, leftValue, rightValue)) {
        return false;
    }

    // 生成乘法指令
    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(mulInst);
    node->val = mulInst;

    return true;
}

/// @brief 浮点数乘法AST节点翻译成线性中间IR（使用已处理的操作数）
bool IRGenerator::ir_fmul_processed(ast_node * node, ast_node * left, ast_node * right)
{
    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 确保操作数是浮点类型
    if (!leftValue->getType()->isFloatType()) {
        leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
        if (!leftValue)
            return false;
    }
    if (!rightValue->getType()->isFloatType()) {
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue)
            return false;
    }

    // 生成浮点乘法指令
    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_F,
                                                        leftValue,
                                                        rightValue,
                                                        FloatType::getTypeFloat());
    node->blockInsts.addInst(mulInst);
    node->val = mulInst;

    return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR（使用已处理的操作数）
bool IRGenerator::ir_div_processed(ast_node * node, ast_node * left, ast_node * right)
{
    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 处理类型转换
    if (!handleArithmeticTypeConversion(node, leftValue, rightValue)) {
        return false;
    }

    // 生成除法指令
    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_I,
                                                        leftValue,
                                                        rightValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(divInst);
    node->val = divInst;

    return true;
}

/// @brief 浮点数除法AST节点翻译成线性中间IR（使用已处理的操作数）
bool IRGenerator::ir_fdiv_processed(ast_node * node, ast_node * left, ast_node * right)
{
    Value * leftValue = nullptr;
    Value * rightValue = nullptr;

    // 左操作数
    if (needsLoad(left->val)) {
        LoadInstruction * loadLeft = new LoadInstruction(module->getCurrentFunction(), left->val, left->val, 4);
        node->blockInsts.addInst(loadLeft);
        leftValue = loadLeft;
    } else {
        leftValue = left->val;
    }

    // 右操作数
    if (needsLoad(right->val)) {
        LoadInstruction * loadRight = new LoadInstruction(module->getCurrentFunction(), right->val, right->val, 4);
        node->blockInsts.addInst(loadRight);
        rightValue = loadRight;
    } else {
        rightValue = right->val;
    }

    // 确保操作数是浮点类型
    if (!leftValue->getType()->isFloatType()) {
        leftValue = convertToFloat(leftValue, module->getCurrentFunction(), node->blockInsts);
        if (!leftValue)
            return false;
    }
    if (!rightValue->getType()->isFloatType()) {
        rightValue = convertToFloat(rightValue, module->getCurrentFunction(), node->blockInsts);
        if (!rightValue)
            return false;
    }

    // 生成浮点除法指令
    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_F,
                                                        leftValue,
                                                        rightValue,
                                                        FloatType::getTypeFloat());
    node->blockInsts.addInst(divInst);
    node->val = divInst;

    return true;
}

/// @brief 预收集函数中的所有局部变量声明
/// @param node 函数体AST节点
/// @param localVars 收集到的局部变量列表
/// @return 收集是否成功
bool IRGenerator::collectLocalVariables(ast_node * node, std::vector<LocalVarInfo> & localVars)
{
    static int varCounter = 0; // 静态计数器，为每个变量分配唯一ID
    return collectLocalVariablesRecursive(node, localVars, varCounter);
}

/// @brief 递归收集局部变量声明的内部实现
/// @param node AST节点
/// @param localVars 收集到的局部变量列表
/// @param varCounter 变量计数器引用
/// @return 收集是否成功
bool IRGenerator::collectLocalVariablesRecursive(ast_node * node,
                                                 std::vector<LocalVarInfo> & localVars,
                                                 int & varCounter)
{
    if (!node) {
        return true;
    }

    // 如果是变量声明语句，收集其中的变量
    if (node->node_type == ast_operator_type::AST_OP_DECL_STMT) {
        for (auto & child: node->sons) {
            if (child->node_type == ast_operator_type::AST_OP_VAR_DECL) {
                // 处理变量声明
                if (child->sons.size() < 2) {
                    printf("Error: Invalid node structure in collectLocalVariables. Expected 2 children, got %zu.\n",
                           child->sons.size());
                    return false;
                }

                ast_node * typeNode = child->sons[0];
                ast_node * varOrAssignNode = child->sons[1];

                if (!typeNode || !varOrAssignNode) {
                    printf("Error: Null typeNode or varOrAssignNode in collectLocalVariables.\n");
                    return false;
                }

                // 处理变量声明（带或不带初始化）
                ast_node * varNode = nullptr;

                if (varOrAssignNode->node_type == ast_operator_type::AST_OP_ASSIGN) {
                    // 带初始化的声明
                    if (varOrAssignNode->sons.size() < 2) {
                        printf("Error: Invalid assignment structure in collectLocalVariables.\n");
                        return false;
                    }
                    varNode = varOrAssignNode->sons[0];
                } else {
                    // 不带初始化的声明
                    varNode = varOrAssignNode;
                }

                if (!varNode) {
                    printf("Error: Null varNode in collectLocalVariables.\n");
                    return false;
                }

                // 处理数组类型
                Type * varType = typeNode->type;
                if (!varNode->sons.empty() && !typeNode->type->isArrayType()) {
                    // 变量节点有子节点，说明是数组声明，需要创建数组类型
                    std::vector<int> dimensions;
                    for (auto dimNode: varNode->sons) {
                        if (dimNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
                            dimensions.push_back(dimNode->integer_val);
                        } else {
                            printf("Error: Non-constant array dimension in variable declaration.\n");
                            return false;
                        }
                    }
                    varType = new ArrayType(typeNode->type, dimensions);
                }

                // 计算对齐大小
                uint32_t alignSize = varType->isArrayType() ? 16 : varType->getSize();

                // 添加到局部变量列表，分配唯一ID
                localVars.emplace_back(varNode->name, varType, alignSize, varNode, ++varCounter);
            }
        }
    }

    // 递归处理子节点
    for (auto & child: node->sons) {
        if (!collectLocalVariablesRecursive(child, localVars, varCounter)) {
            return false;
        }
    }

    return true;
}

/// @brief 在函数开始时生成所有局部变量的alloca指令
/// @param localVars 局部变量列表
/// @param blockInsts 指令序列
/// @return 生成是否成功
bool IRGenerator::generateAllocaInstructions(const std::vector<LocalVarInfo> & localVars, InterCode & blockInsts)
{
    for (const auto & varInfo: localVars) {
        // 变量的类型应该是指向分配类型的指针类型
        Type * allocaType = varInfo.type;             // 要分配的类型
        Type * varType = new PointerType(allocaType); // 变量类型（指针类型）

        Value * varValue = module->newVarValue(varType, varInfo.uniqueName);
        if (!varValue) {
            printf("Error: Failed to allocate variable '%s' (unique: %s) in generateAllocaInstructions.\n",
                   varInfo.name.c_str(),
                   varInfo.uniqueName.c_str());
            return false;
        }

        // 生成alloca指令
        AllocaInstruction * allocaInst =
            new AllocaInstruction(module->getCurrentFunction(), varValue, allocaType, varInfo.alignSize);
        blockInsts.addInst(allocaInst);

        // 设置变量节点的值
        varInfo.varNode->val = varValue;

        // 注意：不在这里进行数组零初始化，让原有的初始化逻辑处理

        // printf("Debug: Generated alloca for variable '%s' (unique: %s) with type %s\n",
        // varInfo.name.c_str(),
        // varInfo.uniqueName.c_str(),
        // allocaType->toString().c_str());
    }

    return true;
}

/// @brief 修改后的变量声明处理，只处理初始化（变量已在函数开始时分配）
/// @param node AST节点
/// @return 翻译是否成功
bool IRGenerator::ir_variable_declare_register_only(ast_node * node)
{
    // 确保节点有两个子节点：类型节点和变量名或赋值节点
    if (node->sons.size() < 2) {
        printf("Error: Invalid node structure in ir_variable_declare_register_only. Expected 2 children, got %zu.\n",
               node->sons.size());
        return false;
    }

    ast_node * typeNode = node->sons[0];
    ast_node * varOrAssignNode = node->sons[1];

    if (!typeNode || !varOrAssignNode) {
        printf("Error: Null typeNode or varOrAssignNode in ir_variable_declare_register_only.\n");
        return false;
    }

    // 处理变量声明（带或不带初始化）
    ast_node * varNode = nullptr;
    ast_node * initExprNode = nullptr;

    if (varOrAssignNode->node_type == ast_operator_type::AST_OP_ASSIGN) {
        // 带初始化的声明
        if (varOrAssignNode->sons.size() < 2) {
            printf("Error: Invalid assignment structure in ir_variable_declare_register_only.\n");
            return false;
        }

        varNode = varOrAssignNode->sons[0];
        initExprNode = varOrAssignNode->sons[1];

        if (!varNode || !initExprNode) {
            printf("Error: Null varNode or initExprNode in assignment.\n");
            return false;
        }
    } else {
        // 不带初始化的声明
        varNode = varOrAssignNode;
    }

    // 变量应该已经在函数开始时分配，从符号表中查找
    Value * varValue = module->findVarValue(varNode->name);
    if (!varValue) {
        printf("Error: Variable '%s' not found in symbol table. It should have been allocated at function start.\n",
               varNode->name.c_str());
        return false;
    }

    // 设置变量节点的值（用于后续引用）
    varNode->val = varValue;

    // 只处理初始化（如果有的话）
    if (initExprNode) {
        // 处理初始化表达式
        if (!ir_visit_ast_node(initExprNode)) {
            printf("Error: Failed to evaluate initialization expression for '%s'.\n", varNode->name.c_str());
            return false;
        }

        Value * initValue = nullptr;

        // 处理不同类型的初始化
        if (initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT) {
            initValue = module->newConstFloat(initExprNode->float_val);
            node->blockInsts.addInst(initExprNode->blockInsts);
        } else if (initExprNode->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
            // 数组访问返回地址，需要加载值
            node->blockInsts.addInst(initExprNode->blockInsts);

            if (initExprNode->val && initExprNode->val->getType()->isPointerType()) {
                const PointerType * ptrType = static_cast<const PointerType *>(initExprNode->val->getType());
                Value * loadResult = module->newVarValue(const_cast<Type *>(ptrType->getPointeeType()));
                LoadInstruction * loadInst =
                    new LoadInstruction(module->getCurrentFunction(), loadResult, initExprNode->val);
                node->blockInsts.addInst(loadInst);
                initValue = loadResult;
            } else {
                initValue = initExprNode->val;
            }
        } else {
            // 其他表达式
            node->blockInsts.addInst(initExprNode->blockInsts);
            initValue = initExprNode->val;
        }

        if (initValue) {
            // 生成store指令进行初始化
            StoreInstruction * storeInst = new StoreInstruction(module->getCurrentFunction(), initValue, varValue);
            node->blockInsts.addInst(storeInst);
        }
    }

    return true;
}

/// @brief 处理一维数组动态初始化
/// @param node AST节点
/// @param arrayVar 数组变量
/// @param arrayType 数组类型
/// @param initExprNode 初始化表达式节点
/// @return 翻译是否成功
bool IRGenerator::handleOneDimensionalDynamicInit(ast_node * node,
                                                  Value * arrayVar,
                                                  ArrayType * arrayType,
                                                  ast_node * initExprNode)
{
    Function * currentFunc = module->getCurrentFunction();

    // printf("Debug: Processing 1D array dynamic initialization for array with %d elements\n",
    // arrayType->getDimensions()[0]);

    // 获取数组维度
    int arraySize = arrayType->getDimensions()[0];

    // 逐个处理初始化元素
    for (size_t i = 0; i < initExprNode->sons.size() && i < static_cast<size_t>(arraySize); ++i) {
        ast_node * elementNode = initExprNode->sons[i];

        // 处理元素表达式
        if (!ir_visit_ast_node(elementNode)) {
            printf("Error: Failed to process initialization element %zu\n", i);
            return false;
        }
        node->blockInsts.addInst(elementNode->blockInsts);

        Value * elementValue = elementNode->val;

        // 如果元素值需要加载（如变量引用），先加载
        if (needsLoad(elementValue)) {
            LoadInstruction * loadInst = new LoadInstruction(currentFunc, elementValue, elementValue, 4);
            node->blockInsts.addInst(loadInst);
            elementValue = loadInst;
        }

        // 生成GEP指令获取数组元素地址
        Value * firstIndex = module->newConstInt(0);                    // 第一个索引总是0（数组基址）
        Value * secondIndex = module->newConstInt(static_cast<int>(i)); // 元素索引

        GetelementptrInstruction * gepInst =
            new GetelementptrInstruction(currentFunc, arrayVar, firstIndex, secondIndex);
        node->blockInsts.addInst(gepInst);

        // 生成store指令将值存储到数组元素
        StoreInstruction * storeInst = new StoreInstruction(currentFunc, elementValue, gepInst);
        node->blockInsts.addInst(storeInst);

        // printf("Debug: Generated store for element %zu\n", i);
    }

    // 如果初始化元素少于数组大小，剩余元素需要初始化为0
    if (initExprNode->sons.size() < static_cast<size_t>(arraySize)) {
        // printf("Debug: Array has %d elements but only %zu initialization values provided, initializing remaining "
        //"elements to 0\n",
        // arraySize,
        // initExprNode->sons.size());

        // 初始化剩余元素为0
        for (size_t i = initExprNode->sons.size(); i < static_cast<size_t>(arraySize); ++i) {
            // 生成GEP指令获取数组元素地址
            Value * firstIndex = module->newConstInt(0);                    // 第一个索引总是0（数组基址）
            Value * secondIndex = module->newConstInt(static_cast<int>(i)); // 元素索引

            GetelementptrInstruction * gepInst =
                new GetelementptrInstruction(currentFunc, arrayVar, firstIndex, secondIndex);
            node->blockInsts.addInst(gepInst);

            // 根据数组元素类型生成相应的0值
            Value * zeroValue = nullptr;
            Type * elementType = arrayType->getElementType();
            if (elementType->isIntegerType()) {
                zeroValue = module->newConstInt(0);
            } else if (elementType->isFloatType()) {
                zeroValue = module->newConstFloat(0.0f);
            } else {
                printf("Error: Unsupported element type for zero initialization\n");
                return false;
            }

            // 生成store指令将0值存储到数组元素
            StoreInstruction * storeInst = new StoreInstruction(currentFunc, zeroValue, gepInst);
            node->blockInsts.addInst(storeInst);

            // printf("Debug: Initialized element %zu to 0\n", i);
        }
    }

    // printf("Debug: Completed 1D array dynamic initialization\n");
    return true;
}

/// @brief 将预分配的变量注册到当前作用域
/// @param name 变量名
/// @param value 变量值
/// @return 注册是否成功
bool IRGenerator::registerVariableToCurrentScope(const std::string & name, Value * value)
{
    return module->registerVariableToCurrentScope(name, value);
}