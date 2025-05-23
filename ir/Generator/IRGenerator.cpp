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

#include "AST.h"
#include "Common.h"
#include "Function.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Module.h"
#include "EntryInstruction.h"
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

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module) : root(_root), module(_module)
{
    /* 叶子节点 */
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

    /* 表达式运算， 加减 */
    ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub;
    ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add;
    ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul;
    ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div;

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
    bool result;

    printf("==== ENTER ir_function_define ====\n");

    // 创建一个函数，用于当前函数处理
    if (module->getCurrentFunction()) {
        // 函数中嵌套定义函数，这是不允许的，错误退出
        // TODO 自行追加语义错误处理
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

    // 创建一个新的函数定义，函数的返回类型设置为type_node里的type
    Function * newFunc = module->newFunction(name_node->name, type_node->type);
    if (!newFunc) {
        printf("Error: Function %s already exists.\n", name_node->name.c_str());
        return false;
    }

    module->setCurrentFunction(newFunc);
    module->enterScope();
    InterCode & irCode = newFunc->getInterCode();

    // 入口label和entry, entry不要了
    LabelInstruction * entryLabelInst = new LabelInstruction(newFunc);
    irCode.addInst(entryLabelInst);
    // irCode.addInst(new EntryInstruction(newFunc));

    // 出口label
    LabelInstruction * exitLabelInst = new LabelInstruction(newFunc);
    newFunc->setExitLabel(exitLabelInst);

    // 处理形参
    result = ir_function_formal_params(param_node);
    if (!result) {
        printf("Error: Failed to process function parameters.\n");
        return false;
    }
    node->blockInsts.addInst(param_node->blockInsts);

    // 新建一个Value，用于保存函数的返回值，如果没有返回值可不用申请
    LocalVariable * retValue = nullptr;
    AllocaInstruction * allocaRet = nullptr;
    StoreInstruction * storeRet = nullptr;
    if (!type_node->type->isVoidType()) {
        // 为所有非void函数创建返回值变量
        retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type, "__ret"));
        allocaRet = new AllocaInstruction(newFunc, retValue, type_node->type, 4);
        irCode.addInst(allocaRet);

        // 只有main函数初始化为0，其他函数不初始化
        if (name_node->name == "main") {
            // 创建一个常量0
            ConstInt * zeroConst = module->newConstInt(0);
            // 创建一个store指令，将0存储到retValue
            storeRet = new StoreInstruction(newFunc, zeroConst, retValue, 4);
            irCode.addInst(storeRet);
        }
    }
    newFunc->setReturnValue(retValue);

    // 处理block
    block_node->needScope = false;
    result = ir_block(block_node);
    if (!result) {
        return false;
    }
    // IR指令追加到当前的节点中
    node->blockInsts.addInst(block_node->blockInsts);

    // 此时，所有指令都加入到当前函数中，也就是node->blockInsts

    // node节点的指令移动到函数的IR指令列表中
    irCode.addInst(node->blockInsts);

    // 现在irCode中，第一条是allocaRet指令，第二条是storeRet指令
    // 然后是处理block部分得到的指令，这些指令的第一部分是对decl-stmt节点的处理，是若干个alloca指令，store指令
    // 如果可能的话，赋值是是变量赋值，会有load指令
    // 所以目前，第二条storeRet指令会夹在第一条alloca指令和后面block的decl-stmt指令中间
    // 但希望实现的效果是store指令在所有alloca指令后面
    // 所以对irCode中的指令进行重新排序,
    // 逻辑是遇到第一条store指令，将其放到最后一条alloca指令后面即可，后面再遇到store不用管了

    //=============指令重排序逻辑开始===================

    auto & insts = irCode.getInsts();
    std::vector<Instruction *> allocaInsts;
    std::vector<Instruction *> initStoreInsts; // 变量初始化相关的store指令
    std::vector<Instruction *> otherInsts;

    // 预分配容量以避免在循环中多次重新分配
    size_t totalInsts = insts.size();
    allocaInsts.reserve(totalInsts / 4);    // 估算alloca指令数量
    initStoreInsts.reserve(totalInsts / 4); // 估算store指令数量
    otherInsts.reserve(totalInsts / 2);     // 估算其他指令数量

    // 找到第一个非ENTRY指令的位置作为插入点
    int insertPos = -1;
    for (size_t i = 0; i < insts.size(); ++i) {
        if (insts[i]->getOp() == IRInstOperator::IRINST_OP_LABEL) {
            // 入口标签之后就是插入点
            insertPos = (int) i + 1;
            break;
        }
    }

    if (insertPos == -1) {
        printf("Error: No entry label found for reordering.\n");
        return false;
    }

    // 收集所有alloca指令的目标变量
    std::vector<Value *> allocaTargets;
    for (size_t i = insertPos; i < insts.size(); ++i) {
        if (dynamic_cast<AllocaInstruction *>(insts[i])) {
            AllocaInstruction * allocaInst = static_cast<AllocaInstruction *>(insts[i]);
            // AllocaInstruction的第一个操作数应该是目标变量
            if (allocaInst->getOperandsNum() > 0) {
                allocaTargets.push_back(allocaInst->getOperand(0));
            }
            allocaInsts.push_back(insts[i]);
        }
    }

    // 分类其他指令
    for (size_t i = insertPos; i < insts.size(); ++i) {
        Instruction * inst = insts[i];

        if (dynamic_cast<AllocaInstruction *>(inst)) {
            // alloca指令已经处理过了
            continue;
        } else if (dynamic_cast<StoreInstruction *>(inst)) {
            StoreInstruction * storeInst = static_cast<StoreInstruction *>(inst);

            // 检查这个store指令是否对应某个alloca指令的初始化
            bool isInitStore = false;
            if (storeInst->getOperandsNum() >= 2) {
                Value * storeSource = storeInst->getOperand(0); // store指令的第一个操作数是源值
                Value * storeTarget = storeInst->getOperand(1); // store指令的第二个操作数是目标

                // 检查目标是否在alloca目标列表中
                bool targetInAllocaList = false;
                for (Value * allocaTarget: allocaTargets) {
                    if (storeTarget == allocaTarget) {
                        targetInAllocaList = true;
                        break;
                    }
                }

                if (targetInAllocaList) {
                    // 进一步检查是否是初始化store
                    // 初始化store的源值应该是：
                    // 1. 常量值（如变量初始化）
                    // 2. 函数形参（如 %0, %1, %2 等，但只对有参数的函数）
                    if (dynamic_cast<ConstInt *>(storeSource)) {
                        // 常量初始化
                        isInitStore = true;
                    } else if (dynamic_cast<FormalParam *>(storeSource)) {
                        // 形参初始化
                        isInitStore = true;
                    } else {
                        // 检查是否是形参值，但需要排除指令结果
                        if (!dynamic_cast<Instruction *>(storeSource)) {
                            // 不是指令结果，再检查名字格式
                            std::string sourceName = storeSource->getIRName();
                            if (sourceName.size() >= 2 && sourceName[0] == '%' && std::isdigit(sourceName[1]) &&
                                sourceName.find_first_not_of("0123456789", 1) == std::string::npos) {
                                // 这是形参（%0, %1, %2 等格式），且不是指令结果
                                isInitStore = true;
                            }
                        }
                        // 如果源值是指令结果（如 load 指令的结果），则不是初始化store
                    }
                }
            }

            if (isInitStore) {
                initStoreInsts.push_back(inst);
            } else {
                otherInsts.push_back(inst);
            }
        } else {
            otherInsts.push_back(inst);
        }
    }

    // 重新构建指令序列
    std::vector<Instruction *> newInsts;

    // 保留前面的ENTRY等指令
    for (int i = 0; i < insertPos; ++i) {
        newInsts.push_back(insts[i]);
    }

    // 先添加所有alloca指令
    for (auto allocaInst: allocaInsts) {
        newInsts.push_back(allocaInst);
    }

    // 再添加所有对应的初始化store指令
    for (auto storeInst: initStoreInsts) {
        newInsts.push_back(storeInst);
    }

    // 最后添加其他指令
    for (auto otherInst: otherInsts) {
        newInsts.push_back(otherInst);
    }

    // 替换原指令序列
    insts = newInsts;

    //=============指令重排序逻辑结尾===================

    // 添加函数出口Label指令
    irCode.addInst(exitLabelInst);

    // 函数出口指令 - 根据函数类型决定如何生成
    if (!type_node->type->isVoidType() && retValue) {
        // 非void函数需要从返回值变量加载值再返回

        // 创建load指令，从返回值变量加载值
        LoadInstruction * loadRet = new LoadInstruction(newFunc, retValue, retValue, 4);
        irCode.addInst(loadRet);

        // 创建返回指令，返回加载的值
        irCode.addInst(new ExitInstruction(newFunc, loadRet));
    } else {
        // void函数
        irCode.addInst(new ExitInstruction(newFunc, nullptr));
    }

    // 恢复成外部函数
    module->setCurrentFunction(nullptr);
    module->leaveScope();

    printf("==== EXIT ir_function_define ====\n");
    printf("Function has %zu instructions\n", irCode.getInsts().size());
    std::string fullIR;
    newFunc->toString(fullIR);
    printf("Final IR after rename:\n%s\n", fullIR.c_str());
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
        Value * paramVar = module->newVarValue(typeNode->type, nameNode->name);

        if (!paramVar) {
            printf("Error: Failed to create local variable for parameter '%s'.\n", nameNode->name.c_str());
            return false;
        }

        // 转换为 LocalVariable 类型
        LocalVariable * localParamVar = static_cast<LocalVariable *>(paramVar);

        // 创建 alloca 指令，添加4字节对齐
        AllocaInstruction * allocaInst = new AllocaInstruction(currentFunc, paramVar, typeNode->type, 4);
        currentFunc->getInterCode().addInst(allocaInst);

        // 创建 store 指令，将形参的值存储到局部变量，添加4字节对齐

        // 设置形参节点的值为创建的局部变量（函数体内使用这个变量）
        nameNode->val = paramVar;

        // 将 (形参, 局部变量) 对添加到列表中
        paramPairs.emplace_back(param, localParamVar);
    }

    // 再store所有形参
    for (auto & pair: paramPairs) {
        StoreInstruction * storeInst = new StoreInstruction(currentFunc, pair.first, pair.second, 4);
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

            // 处理参数值
            if (needsLoad(temp->val)) {
                // 参数是变量，需要加载
                LoadInstruction * loadParam = new LoadInstruction(currentFunc, temp->val, temp->val, 4);
                node->blockInsts.addInst(loadParam);
                paramValue = loadParam;
            } else {
                // 参数是常量或表达式结果，直接使用
                paramValue = temp->val;
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
    }

    // 离开作用域
    if (node->needScope) {
        module->leaveScope();
    }

    return true;
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

/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 赋值节点，自右往左运算

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

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    // TODO real number add

    Value * rightValue = nullptr;

    // 处理右操作数
    if (needsLoad(right->val)) {
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

    // 创建 store 指令，将右侧值存储到左侧变量
    StoreInstruction * storeInst = new StoreInstruction(module->getCurrentFunction(), rightValue, left->val, 4);

    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(storeInst);

    node->val = rightValue;

    return true;
}

/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
    ast_node * right = nullptr;

    // return语句可能没有表达式，也可能有，因此这里必须进行区分判断
    if (!node->sons.empty()) {
        ast_node * son_node = node->sons[0];

        // 返回的表达式的指令保存在right节点中
        right = ir_visit_ast_node(son_node);
        if (!right) {
            // 某个变量没有定值
            return false;
        }
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    Function * currentFunc = module->getCurrentFunction();
    Value * returnValue = nullptr;

    // 处理返回值
    if (right && right->val) {
        if (needsLoad(right->val)) {
            // 变量需要load
            LoadInstruction * loadRight = new LoadInstruction(currentFunc, right->val, right->val, 4);
            node->blockInsts.addInst(right->blockInsts);
            node->blockInsts.addInst(loadRight);
            returnValue = loadRight;
        } else {
            // 常量或表达式结果
            node->blockInsts.addInst(right->blockInsts);
            returnValue = right->val;
        }

        // 如果函数有返回值变量，则将返回值存储到函数的返回值变量中
        LocalVariable * retVar = currentFunc->getReturnValue();
        if (retVar) {
            // 检查是否已经有相同的store指令
            bool needStore = true;
            if (dynamic_cast<ConstInt *>(returnValue)) {
                ConstInt * constRet = static_cast<ConstInt *>(returnValue);
                if (constRet->getVal() == 0) {
                    // 如果返回常量0，可能已经有默认的store 0指令，跳过
                    needStore = false;
                }
            }

            if (needStore) {
                StoreInstruction * storeRetValue = new StoreInstruction(currentFunc, returnValue, retVar, 4);
                node->blockInsts.addInst(storeRetValue);
            }
        } else {
        }
    }

    // 获取函数出口标签
    Instruction * exitLabel = currentFunc->getExitLabel();
    if (!exitLabel) {
        printf("Error: No exit label defined for function in ir_return.\n");
        return false;
    }

    // 创建跳转到函数出口的指令，而不是创建返回指令
    GotoInstruction * gotoExit = new GotoInstruction(currentFunc, exitLabel);
    node->blockInsts.addInst(gotoExit);

    // 设置节点值为返回值
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
    ConstInt * val;

    // 新建一个整数常量Value
    val = module->newConstInt((int32_t) node->integer_val);

    node->val = val;

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
    // 共有两个孩子，第一个类型，第二个变量名

    // TODO 这里可强化类型等检查

    // 确保节点有两个子节点：类型节点和变量名或赋值节点
    if (node->sons.size() < 2) {
        printf("Error: Invalid node structure in ir_variable_declare.\n");
        return false;
    }

    ast_node * typeNode = node->sons[0];
    ast_node * varOrAssignNode = node->sons[1];

    if (!typeNode || !varOrAssignNode) {
        printf("Error: Null typeNode or varOrAssignNode in ir_variable_declare.\n");
        return false;
    }

    // 检查当前是否在全局作用域（函数外部）
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        // 全局变量处理
        return ir_global_variable_declare(node, typeNode, varOrAssignNode);
    }

    // 如果是赋值节点（AST_OP_ASSIGN）
    if (varOrAssignNode->node_type == ast_operator_type::AST_OP_ASSIGN) {
        // 获取赋值节点的子节点：变量名和初值表达式
        ast_node * varNode = varOrAssignNode->sons[0];
        ast_node * initExprNode = varOrAssignNode->sons[1];

        if (!varNode || !initExprNode) {
            printf("Error: Invalid assignment structure in ir_variable_declare.\n");
            return false;
        }

        // 在符号表中为变量分配 Value
        Value * varValue = module->newVarValue(typeNode->type, varNode->name);
        if (!varValue) {
            printf("Error: Failed to allocate variable in ir_variable_declare.\n");
            return false;
        }
        // 创建 alloca 指令，为变量分配栈空间
        AllocaInstruction * allocaInst =
            new AllocaInstruction(module->getCurrentFunction(), varValue, typeNode->type, 4);

        // 将变量名节点的 Value 设置为分配的 Value
        varNode->val = varValue;

        // 计算初值表达式
        if (!ir_visit_ast_node(initExprNode)) {
            printf("Error: Failed to evaluate initialization expression.\n");
            return false;
        }

        Value * initValue = nullptr;

        // 处理初始值
        if (needsLoad(initExprNode->val)) {
            // 初始值是变量，需要加载
            LoadInstruction * loadInit =
                new LoadInstruction(module->getCurrentFunction(), initExprNode->val, initExprNode->val, 4);
            node->blockInsts.addInst(initExprNode->blockInsts);
            node->blockInsts.addInst(loadInit);
            initValue = loadInit;
        } else {
            // 初始值是常量或表达式结果，直接使用
            node->blockInsts.addInst(initExprNode->blockInsts);
            initValue = initExprNode->val;
        }

        // 创建 store 指令，将初始值存储到变量
        StoreInstruction * storeInst = new StoreInstruction(module->getCurrentFunction(), initValue, varValue, 4);

        node->blockInsts.addInst(allocaInst);

        // node->blockInsts.addInst(initExprNode->blockInsts);

        node->blockInsts.addInst(storeInst);

    } else {
        // 如果是普通变量声明（没有初值）
        ast_node * varNode = varOrAssignNode;

        // 在符号表中为变量分配 Value
        Value * varValue = module->newVarValue(typeNode->type, varNode->name);

        // 创建 alloca 指令，为变量分配栈空间
        AllocaInstruction * allocaInst =
            new AllocaInstruction(module->getCurrentFunction(), varValue, typeNode->type, 4);

        // 添加指令到当前节点
        node->blockInsts.addInst(allocaInst);

        if (!varValue) {
            printf("Error: Failed to allocate variable in ir_variable_declare.\n");
            return false;
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

        // 处理初值
        // TODO: 目前只支持整型常量初值，要加浮点数
        Value * initValue = nullptr;
        if (initExprNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
            // 整数常量初值
            initValue = module->newConstInt(initExprNode->integer_val);
        } else {
            printf("Error: Global variable initialization only supports constants.\n");
            return false;
        }

        // 使用公有的 newVarValue 方法创建全局变量
        // 由于currentFunc为nullptr，会自动调用newGlobalVariable
        Value * globalVar = module->newVarValue(typeNode->type, varNode->name);
        if (!globalVar) {
            printf("Error: Failed to create global variable.\n");
            return false;
        }

        // 将全局变量转换为 GlobalVariable 类型并设置初值
        GlobalVariable * globalVariable = static_cast<GlobalVariable *>(globalVar);
        // TODO: 需要在 GlobalVariable 类中添加 setInitValue 方法
        globalVariable->setInitValue(initValue);

        // 设置节点的Value
        varNode->val = globalVar;
        node->val = globalVar;

        return true;

    } else {
        // 如果是普通全局变量声明（没有初值）
        ast_node * varNode = varOrAssignNode;

        // 使用公有的 newVarValue 方法创建全局变量
        Value * globalVar = module->newVarValue(typeNode->type, varNode->name);
        if (!globalVar) {
            printf("Error: Failed to create global variable.\n");
            return false;
        }

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

    // 创建循环的入口、条件、和退出标签
    LabelInstruction * entryLabel = new LabelInstruction(currentFunc);
    LabelInstruction * condLabel = new LabelInstruction(currentFunc);
    LabelInstruction * bodyLabel = new LabelInstruction(currentFunc);
    LabelInstruction * exitLabel = new LabelInstruction(currentFunc);

    // 条件检查标签和推出标签压栈
    loopLabelStack.push({condLabel, exitLabel});

    // 添加入口标签
    node->blockInsts.addInst(entryLabel);

    // 跳转到条件检查
    node->blockInsts.addInst(new GotoInstruction(currentFunc, condLabel));

    // 条件检查标签
    node->blockInsts.addInst(condLabel);

    // 生成条件表达式的 IR
    if (!ir_visit_ast_node(condNode)) {
        return false;
    }
    node->blockInsts.addInst(condNode->blockInsts);

    // 条件跳转指令：条件为真跳转到循环体，为假跳转到退出标签
    node->blockInsts.addInst(new BranchInstruction(currentFunc, condNode->val, bodyLabel, exitLabel));

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

    // 创建条件、then分支、else分支（可选）和结束标签
    LabelInstruction * condLabel = new LabelInstruction(currentFunc);
    LabelInstruction * thenLabel = new LabelInstruction(currentFunc);
    LabelInstruction * elseLabel = elseNode ? new LabelInstruction(currentFunc) : nullptr;
    LabelInstruction * endLabel = new LabelInstruction(currentFunc);

    // 条件检查标签
    node->blockInsts.addInst(condLabel);

    // 生成条件表达式的 IR
    if (!ir_visit_ast_node(condNode)) {
        return false;
    }
    node->blockInsts.addInst(condNode->blockInsts);

    // 条件跳转指令：条件为真跳转到then分支，为假跳转到else分支或结束标签
    node->blockInsts.addInst(
        new BranchInstruction(currentFunc, condNode->val, thenLabel, elseLabel ? elseLabel : endLabel));

    // then分支标签
    node->blockInsts.addInst(thenLabel);

    // 生成then分支的 IR
    if (!ir_visit_ast_node(thenNode)) {
        return false;
    }
    node->blockInsts.addInst(thenNode->blockInsts);

    // 跳转到结束标签
    node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));

    // else分支标签（如果存在）
    if (elseNode) {
        node->blockInsts.addInst(elseLabel);

        // 生成else分支的 IR
        if (!ir_visit_ast_node(elseNode)) {
            return false;
        }
        node->blockInsts.addInst(elseNode->blockInsts);

        // 跳转到结束标签
        node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));
    }

    // 结束标签
    node->blockInsts.addInst(endLabel);

    return true;
}

bool IRGenerator::ir_break(ast_node * node)
{
    if (loopLabelStack.empty()) {
        printf("Error: break statement not inside a loop.\n");
        return false;
    }

    // 获取当前循环的退出标签
    LabelInstruction * exitLabel = loopLabelStack.top().exitLabel;

    // 生成跳转到退出标签的指令
    node->blockInsts.addInst(new GotoInstruction(module->getCurrentFunction(), exitLabel));

    return true;
}

bool IRGenerator::ir_continue(ast_node * node)
{
    if (loopLabelStack.empty()) {
        printf("Error: continue statement not inside a loop.\n");
        return false;
    }

    // 获取当前循环的条件检查标签
    LabelInstruction * condLabel = loopLabelStack.top().condLabel;

    // 生成跳转到条件检查标签的指令
    node->blockInsts.addInst(new GotoInstruction(module->getCurrentFunction(), condLabel));

    return true;
}

bool IRGenerator::needsLoad(Value * val)
{
    // 如果值为空，不需要加载
    if (!val)
        return false;

    // 如果是指令结果，不需要加载
    if (dynamic_cast<Instruction *>(val) != nullptr) {
        return false;
    }

    // 如果是常量，不需要加载
    if (dynamic_cast<ConstInt *>(val) != nullptr) {
        return false;
    }

    // 如果是形参，不需要加载
    if (dynamic_cast<FormalParam *>(val) != nullptr) {
        return false;
    }

    // 其他情况（如局部变量、全局变量）需要加载
    return true;
}