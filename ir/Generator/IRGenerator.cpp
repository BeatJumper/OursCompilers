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
#include "XorInstruction.h"
#include "ZextInstruction.h"
#include "GetelementptrInstruction.h"
#include "BitcastInstruction.h"
#include "MemcpyInstruction.h"
#include "SextInstruction.h"
#include "SitofpInstruction.h"
#include "FptosiInstruction.h"
#include "PointerType.h"
#include "ConstFloat.h"

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
            // 根据返回类型创建相应的常量0
            Value * zeroConst = nullptr;
            if (type_node->type->isFloatType()) {
                zeroConst = module->newConstFloat(0.0f);
            } else {
                zeroConst = module->newConstInt(0);
            }
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

            // 检查参数是否是数组类型，如果是数组需要传递首地址
            if (temp->val->getType()->isArrayType()) {
                // 数组参数：需要获取数组的首地址
                // 使用 getelementptr 获取数组首元素地址
                ConstInt * zeroConst = module->newConstInt(0);
                ConstInt * zeroConst2 = module->newConstInt(0);

                GetelementptrInstruction * gepInst =
                    new GetelementptrInstruction(currentFunc, temp->val, zeroConst, zeroConst2);
                node->blockInsts.addInst(gepInst);
                paramValue = gepInst;
            } else if (needsLoad(temp->val)) {
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

bool IRGenerator::ir_add_or_fadd(ast_node * node)
{
    // 先递归处理左右操作数，获取它们的值类型
    ast_node * leftNode = node->sons[0];
    ast_node * rightNode = node->sons[1];

    ast_node * left = ir_visit_ast_node(leftNode);
    ast_node * right = ir_visit_ast_node(rightNode);

    if (!left || !right || !left->val || !right->val) {
        printf("Debug: Failed to process operands in ir_add_or_fadd\n");
        return false;
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
        printf("Debug: Right operand type: %s\n", rightValue->getType()->toString().c_str());
        printf("Debug: Right operand IR name: %s\n", rightValue->getIRName().c_str());
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
                LoadInstruction * loadRight =
                    new LoadInstruction(currentFunc, right->val, right->val, right->val->getType()->getSize());
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
        // 检查是否需要存储返回值（避免重复存储默认值）
        bool needStore = true;
        if (ConstInt * constInt = dynamic_cast<ConstInt *>(returnValue)) {
            if (constInt->getVal() == 0) {
                // 跳过默认值0的存储
                needStore = false;
            }
        } else if (ConstFloat * constFloat = dynamic_cast<ConstFloat *>(returnValue)) {
            if (constFloat->getVal() == 0.0f) {
                // 跳过默认值0.0的存储
                needStore = false;
            }
        }

        if (needStore) {
            StoreInstruction * storeRet = new StoreInstruction(currentFunc, returnValue, retVar, returnType->getSize());
            node->blockInsts.addInst(storeRet);
        }
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
            // 十六进制数字
            value = std::stoi(numStr, nullptr, 16);
        } else if (numStr.size() >= 2 && numStr[0] == '0' && numStr[1] >= '0' && numStr[1] <= '7') {
            // 八进制数字（以0开头且第二个字符是八进制数字）
            value = std::stoi(numStr, nullptr, 8);
        } else {
            // 十进制数字
            value = std::stoi(numStr, nullptr, 10);
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

    // 检查当前是否在全局作用域
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        return ir_global_variable_declare(node, typeNode, varOrAssignNode);
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

    // 检查是否是数组类型
    if (typeNode->type->isArrayType() ||
        (initExprNode && initExprNode->node_type == ast_operator_type::AST_OP_ARRAY_INIT)) {
        return ir_array_variable_declare_with_init(node, typeNode, varNode, initExprNode);
    }

    // 检查变量节点是否有维度信息（数组声明但没有初始化）
    if (!varNode->sons.empty()) {
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

        // 创建数组类型
        ArrayType * arrayType = new ArrayType(typeNode->type, dimensions);

        // 更新类型节点
        typeNode->type = arrayType;

        // 调用数组处理函数
        return ir_array_variable_declare_with_init(node, typeNode, varNode, initExprNode);
    }

    // 为变量分配Value和栈空间
    Value * varValue = module->newVarValue(typeNode->type, varNode->name);
    if (!varValue) {
        printf("Error: Failed to allocate variable '%s' in ir_variable_declare.\n", varNode->name.c_str());
        return false;
    }

    // 计算对齐大小（基本类型使用类型大小，数组使用16字节对齐）
    uint32_t alignSize = typeNode->type->isArrayType() ? 16 : typeNode->type->getSize();

    AllocaInstruction * allocaInst = new AllocaInstruction(currentFunc, varValue, typeNode->type, alignSize);
    node->blockInsts.addInst(allocaInst);
    varNode->val = varValue;

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

            LoadInstruction * loadInit =
                new LoadInstruction(currentFunc, initExprNode->val, initExprNode->val, elementType->getSize());
            node->blockInsts.addInst(loadInit);
            initValue = loadInit;
        }
        // 处理需要加载的情况
        else if (needsLoad(initExprNode->val)) {
            LoadInstruction * loadInit = new LoadInstruction(currentFunc,
                                                             initExprNode->val,
                                                             initExprNode->val,
                                                             initExprNode->val->getType()->getSize());
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

        // 处理初值
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
    LabelInstruction * thenLabel = new LabelInstruction(currentFunc);
    LabelInstruction * elseLabel = elseNode ? new LabelInstruction(currentFunc) : nullptr;
    LabelInstruction * endLabel = new LabelInstruction(currentFunc);

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

    // 生成负号指令：0 - operand
    ConstInt * zeroConst = module->newConstInt(0);
    BinaryInstruction * negInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        zeroConst,
                                                        operandValue,
                                                        IntegerType::getTypeInt());
    node->blockInsts.addInst(negInst);
    node->val = negInst;

    return true;
}

/// @brief 逻辑非运算符AST节点翻译成线性中间IR（兼容旧接口）
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_not(ast_node * node)
{
    // 这个方法不应该被直接调用，因为逻辑表达式需要外部提供真假出口
    // 如果被调用，说明逻辑表达式被错误地用作普通表达式
    printf("Error: Logical NOT expression cannot be used as regular expression.\n");
    return false;
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

    // 如果是i32类型，转换为i1
    ConstInt * zeroConst = module->newConstInt(0);
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
    LabelInstruction * rightLabel = new LabelInstruction(currentFunc); // L3

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
    LabelInstruction * rightLabel = new LabelInstruction(currentFunc); // L3

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
    // 只支持简单的整数字面量作为全局常量初值
    if (initExprNode->node_type != ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
        printf("Error: Global constant must be initialized with integer literal.\n");
        return false;
    }

    // 解析常量值
    std::string numStr = initExprNode->name;
    int32_t constValue = 0;

    try {
        if (numStr.size() >= 2 && (numStr.substr(0, 2) == "0x" || numStr.substr(0, 2) == "0X")) {
            constValue = std::stoi(numStr, nullptr, 16);
        } else if (numStr.size() >= 2 && numStr[0] == '0' && numStr[1] >= '0' && numStr[1] <= '7') {
            constValue = std::stoi(numStr, nullptr, 8);
        } else {
            constValue = std::stoi(numStr, nullptr, 10);
        }
    } catch (const std::exception & e) {
        printf("Error: Failed to parse constant literal '%s': %s\n", numStr.c_str(), e.what());
        return false;
    }

    // 创建常量值
    ConstInt * constInt = module->newConstInt(constValue);
    if (!constInt) {
        printf("Error: Failed to create constant value.\n");
        return false;
    }

    // 简化处理：全局常量直接使用常量值，不创建变量
    // 将常量添加到符号表中
    // 修复：使用 newGlobalConstant 而不是 newVarValue
    // 修复：直接使用 newVarValue 来创建全局常量
    // 当 currentFunc 为 nullptr 时，会自动创建全局变量
    Value * globalConst = module->newVarValue(typeNode->type, nameNode->name);
    if (!globalConst) {
        printf("Error: Failed to create global constant '%s'.\n", nameNode->name.c_str());
        return false;
    }

    // 将其转换为 GlobalVariable 并设置初值
    GlobalVariable * globalVar = static_cast<GlobalVariable *>(globalConst);
    globalVar->setInitValue(constInt);
    globalVar->setConstant(true);

    // 设置节点值
    nameNode->val = globalConst;
    node->val = globalConst;

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
    Value * constVar = module->newVarValue(typeNode->type, nameNode->name);
    if (!constVar) {
        printf("Error: Failed to create constant variable '%s'.\n", nameNode->name.c_str());
        return false;
    }

    // 创建alloca指令
    AllocaInstruction * allocaInst = new AllocaInstruction(module->getCurrentFunction(), constVar, typeNode->type, 4);
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
        arrayVar = module->findVarValue(arrayNode->name);
        if (!arrayVar) {
            printf("Error: Array variable %s not found.\n", arrayNode->name.c_str());
            return false;
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

    // 创建常量0用于第一个索引
    ConstInt * zeroConst = module->newConstInt(0);

    // 生成getelementptr指令
    GetelementptrInstruction * gepInst =
        new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, zeroConst, indexValue);
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

    printf("ir_array_init: Processing %zu elements, elementType = %s\n",
           node->sons.size(),
           elementType ? elementType->toString().c_str() : "null");

    for (auto son: node->sons) {
        if (son->node_type == ast_operator_type::AST_OP_ARRAY_INIT) {
            // 嵌套的数组初始化列表 - 传递相同的元素类型
            son->type = elementType;
        }

        if (!ir_visit_ast_node(son)) {
            return false;
        }
        node->blockInsts.addInst(son->blockInsts);

        Value * initVal = son->val;

        // 对于常量值，不需要load
        if (dynamic_cast<ConstInt *>(initVal) || dynamic_cast<ConstFloat *>(initVal)) {
            initValues.push_back(initVal);
        } else if (dynamic_cast<GlobalVariable *>(initVal)) {
            // 这是嵌套数组的全局常量，直接使用
            initValues.push_back(initVal);
        } else if (needsLoad(initVal)) {
            LoadInstruction * loadInst = new LoadInstruction(currentFunc, initVal, initVal, 4);
            node->blockInsts.addInst(loadInst);
            initValues.push_back(loadInst);
        } else {
            initValues.push_back(initVal);
        }
    }

    printf("ir_array_init: Initializing array with %zu elements.\n", initValues.size());

    // 确定数组类型
    ArrayType * arrayType = nullptr;

    if (!initValues.empty() && dynamic_cast<GlobalVariable *>(initValues[0])) {
        // 多维数组：子元素是全局常量数组
        Type * innerType = initValues[0]->getType();
        std::vector<int> dimensions = {static_cast<int>(initValues.size())};
        arrayType = new ArrayType(innerType, dimensions);

        printf("Debug: Created nested array type [%d x %s]\n", dimensions[0], innerType->toString().c_str());
    } else {
        // 一维数组：子元素是基础类型常量
        std::vector<int> dimensions = {static_cast<int>(initValues.size())};
        arrayType = new ArrayType(elementType, dimensions);

        printf("Debug: Created simple array type [%d x %s]\n", dimensions[0], elementType->toString().c_str());
    }

    // 创建带有完整初始值的全局常量数组
    GlobalVariable * constArray = module->newGlobalConstArray(arrayType);

    // 设置初始值列表
    constArray->setInitValueList(initValues);

    node->val = constArray;

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
    GlobalVariable * constArray = nullptr;

    // 检查是否已经是数组类型
    printf("Debug: typeNode->type->isArrayType() = %s\n", typeNode->type->isArrayType() ? "true" : "false");
    printf("Debug: initExprNode = %p\n", initExprNode);

    if (typeNode->type->isArrayType()) {
        printf("Debug: Using existing array type\n");
        arrayType = static_cast<ArrayType *>(typeNode->type);

        // 如果有初始化表达式，仍然需要处理
        if (initExprNode) {
            // 获取数组元素类型
            Type * elementType = arrayType->getElementType();
            initExprNode->type = elementType;

            // 处理数组初始化列表，调用注册的 ir_array_init 函数
            printf("Debug: About to process initExprNode, type = %d\n", (int) initExprNode->node_type);
            if (!ir_visit_ast_node(initExprNode)) {
                printf("Error: ir_visit_ast_node failed for initExprNode\n");
                return false;
            }
            printf("Debug: After processing initExprNode, val = %p\n", initExprNode->val);
            node->blockInsts.addInst(initExprNode->blockInsts);
        }
    } else {
        // 如果有初始化表达式，从初始化列表推导数组大小
        if (initExprNode) {
            // ✅ 设置元素类型，供 ir_array_init 使用
            initExprNode->type = typeNode->type;

            // 处理数组初始化列表，调用注册的 ir_array_init 函数
            printf("Debug: About to process initExprNode, type = %d\n", (int) initExprNode->node_type);
            if (!ir_visit_ast_node(initExprNode)) {
                printf("Error: ir_visit_ast_node failed for initExprNode\n");
                return false;
            }
            printf("Debug: After processing initExprNode, val = %p\n", initExprNode->val);
            node->blockInsts.addInst(initExprNode->blockInsts);

            // 从初始化列表获取数组大小
            int arraySize = initExprNode->sons.size();
            std::vector<int> dimensions = {arraySize};
            arrayType = new ArrayType(typeNode->type, dimensions);
        } else {
            printf("Error: Array type expected but not found.\n");
            return false;
        }
    }

    // 创建局部数组变量
    Value * arrayVar = module->newVarValue(arrayType, varNode->name);
    if (!arrayVar) {
        printf("Error: Failed to create array variable.\n");
        return false;
    }

    // 创建 alloca 指令，为数组分配栈空间（16字节对齐）
    AllocaInstruction * allocaInst = new AllocaInstruction(currentFunc, arrayVar, arrayType, 16);
    node->blockInsts.addInst(allocaInst);

    // 如果有初始化表达式，处理初始化
    if (initExprNode) {
        // 获取常量数组（由 ir_array_init 创建）
        printf("Debug: initExprNode->val = %p\n", initExprNode->val);
        if (initExprNode->val) {
            printf("Debug: initExprNode->val type = %s\n", initExprNode->val->getType()->toString().c_str());
        }
        constArray = static_cast<GlobalVariable *>(initExprNode->val);
        if (!constArray) {
            printf("Error: Failed to get constant array from initialization.\n");
            return false;
        }

        // 生成 memcpy 指令将常量数组复制到局部数组

        // 1. 将局部数组转换为 i8*
        BitcastInstruction * destCast = new BitcastInstruction(currentFunc, arrayVar, module->getI8PtrType());
        node->blockInsts.addInst(destCast);

        // 2. 将常量数组转换为 i8*
        BitcastInstruction * srcCast = new BitcastInstruction(currentFunc, constArray, module->getI8PtrType());
        node->blockInsts.addInst(srcCast);

        // 3. 计算拷贝大小
        Type * elementType = arrayType->getElementType();
        int totalElements = arrayType->getTotalElements();
        int elemSize = elementType->getSize();
        ConstInt * sizeConst = module->newConstInt(totalElements * elemSize);

        // 4. 生成 memcpy 指令
        MemcpyInstruction * memcpyInst = new MemcpyInstruction(currentFunc, destCast, srcCast, sizeConst, false);
        node->blockInsts.addInst(memcpyInst);
    }
    // 如果没有初始化表达式，数组将保持未初始化状态（这是正常的）

    // 设置变量节点的值
    varNode->val = arrayVar;

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
