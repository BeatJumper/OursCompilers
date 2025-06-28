///
/// @file DeadCodeElimination.h
/// @brief 死代码消除优化模块头文件
/// @author kyk
/// @version 1.0
/// @date 2024-12-26
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-12-26 <td>1.0     <td>kyk     <td>新建
/// </table>
///

#pragma once

#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>

#include "Function.h"
#include "Instruction.h"
#include "IRCode.h"
#include "LabelInstruction.h"
#include "GotoInstruction.h"
#include "BranchInstruction.h"
#include "ExitInstruction.h"

/// @brief 死代码消除优化类
class DeadCodeElimination {

public:
    /// @brief 构造函数
    DeadCodeElimination() = default;

    /// @brief 析构函数
    ~DeadCodeElimination() = default;

    /// @brief 对函数进行死代码消除优化
    /// @param func 要优化的函数
    /// @return 是否进行了优化（移除了死代码）
    bool eliminateDeadCode(Function * func);

    /// @brief 对指令序列进行死代码消除优化
    /// @param code 要优化的指令序列
    /// @return 是否进行了优化（移除了死代码）
    bool eliminateDeadCode(InterCode & code);

private:
    /// @brief 基本块结构
    struct BasicBlock {
        std::vector<Instruction *> instructions; ///< 基本块中的指令
        std::set<BasicBlock *> predecessors;     ///< 前驱基本块
        std::set<BasicBlock *> successors;       ///< 后继基本块
        LabelInstruction * label;                ///< 基本块的标签（可能为nullptr）
        bool reachable;                          ///< 是否可达

        BasicBlock() : label(nullptr), reachable(false)
        {}
    };

    /// @brief 构建控制流图
    /// @param instructions 指令序列
    /// @return 基本块列表
    std::vector<BasicBlock *> buildControlFlowGraph(const std::vector<Instruction *> & instructions);

    /// @brief 标记可达的基本块
    /// @param blocks 基本块列表
    void markReachableBlocks(std::vector<BasicBlock *> & blocks);

    /// @brief 移除不可达的基本块
    /// @param blocks 基本块列表
    /// @param instructions 原始指令序列
    /// @return 优化后的指令序列
    std::vector<Instruction *> removeUnreachableBlocks(const std::vector<BasicBlock *> & blocks,
                                                       const std::vector<Instruction *> & instructions);

    /// @brief 移除冗余的跳转指令
    /// @param instructions 指令序列
    /// @return 是否进行了优化
    bool removeRedundantJumps(std::vector<Instruction *> & instructions);

    /// @brief 检查指令是否是终结指令（跳转、分支、返回）
    /// @param inst 指令
    /// @return 是否是终结指令
    bool isTerminatorInstruction(Instruction * inst);

    /// @brief 检查指令是否是return指令
    /// @param inst 指令
    /// @return 是否是return指令
    bool isReturnInstruction(Instruction * inst);

    /// @brief 检查指令是否是标签指令
    /// @param inst 指令
    /// @return 是否是标签指令
    bool isLabelInstruction(Instruction * inst);

    /// @brief 获取跳转指令的目标标签
    /// @param inst 跳转指令
    /// @return 目标标签，如果不是跳转指令则返回nullptr
    LabelInstruction * getJumpTarget(Instruction * inst);

    /// @brief 获取分支指令的目标标签
    /// @param inst 分支指令
    /// @param getTrueTarget 是否获取真分支目标（否则获取假分支目标）
    /// @return 目标标签，如果不是分支指令则返回nullptr
    LabelInstruction * getBranchTarget(Instruction * inst, bool getTrueTarget);

    /// @brief 查找标签指令在指令序列中的位置
    /// @param instructions 指令序列
    /// @param label 要查找的标签
    /// @return 标签在序列中的索引，如果未找到则返回-1
    int findLabelIndex(const std::vector<Instruction *> & instructions, LabelInstruction * label);

    /// @brief 检查跳转是否是冗余的（跳转到下一条指令）
    /// @param instructions 指令序列
    /// @param index 跳转指令的索引
    /// @return 是否是冗余跳转
    bool isRedundantJump(const std::vector<Instruction *> & instructions, size_t index);

    /// @brief 清理基本块内存
    /// @param blocks 基本块列表
    void cleanupBlocks(std::vector<BasicBlock *> & blocks);
};
