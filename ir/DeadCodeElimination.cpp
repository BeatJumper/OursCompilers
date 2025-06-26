///
/// @file DeadCodeElimination.cpp
/// @brief 死代码消除优化模块实现文件
/// @author AI Assistant
/// @version 1.0
/// @date 2024-12-26
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-12-26 <td>1.0     <td>AI      <td>新建
/// </table>
///

#include "DeadCodeElimination.h"
#include <algorithm>
#include <queue>
#include <iostream>

/// @brief 对函数进行死代码消除优化
/// @param func 要优化的函数
/// @return 是否进行了优化（移除了死代码）
bool DeadCodeElimination::eliminateDeadCode(Function * func)
{
    if (!func) {
        return false;
    }

    InterCode & code = func->getInterCode();
    return eliminateDeadCode(code);
}

/// @brief 对指令序列进行死代码消除优化
/// @param code 要优化的指令序列
/// @return 是否进行了优化（移除了死代码）
bool DeadCodeElimination::eliminateDeadCode(InterCode & code)
{
    std::vector<Instruction *> & instructions = code.getInsts();
    if (instructions.empty()) {
        return false;
    }

    bool optimized = false;

    // 第一步：移除冗余的跳转指令
    if (removeRedundantJumps(instructions)) {
        optimized = true;
        std::cout << "Dead code elimination: Removed redundant jumps" << std::endl;
    }

    // 第二步：构建控制流图并移除不可达代码
    std::vector<BasicBlock *> blocks = buildControlFlowGraph(instructions);
    if (!blocks.empty()) {
        // 标记可达的基本块
        markReachableBlocks(blocks);

        // 移除不可达的基本块
        std::vector<Instruction *> newInstructions = removeUnreachableBlocks(blocks, instructions);

        if (newInstructions.size() < instructions.size()) {
            instructions = newInstructions;
            optimized = true;
            std::cout << "Dead code elimination: Removed unreachable blocks" << std::endl;
        }

        // 清理内存
        cleanupBlocks(blocks);
    }

    // 第三步：再次移除冗余跳转（可能在移除不可达代码后产生新的冗余跳转）
    if (removeRedundantJumps(instructions)) {
        optimized = true;
        std::cout << "Dead code elimination: Removed additional redundant jumps" << std::endl;
    }

    return optimized;
}

/// @brief 移除冗余的跳转指令
/// @param instructions 指令序列
/// @return 是否进行了优化
bool DeadCodeElimination::removeRedundantJumps(std::vector<Instruction *> & instructions)
{
    bool optimized = false;
    std::vector<Instruction *> newInstructions;

    for (int i = 0; i < instructions.size(); ++i) {
        Instruction * inst = instructions[i];
        bool shouldKeep = true;

        // 暂时禁用冗余跳转消除，以保护break和continue语句
        // TODO: 实现更智能的跳转分析，区分结构性跳转和控制流跳转
        /*
        // 检查是否是冗余的跳转指令
        if (inst->getOp() == IRInstOperator::IRINST_OP_GOTO) {
            // 检查是否跳转到下一条指令
            if (isRedundantJump(instructions, i)) {
                GotoInstruction * gotoInst = dynamic_cast<GotoInstruction *>(inst);
                std::cout << "Removing redundant jump at position " << i;
                if (gotoInst && gotoInst->getTarget()) {
                    std::cout << " (target: " << gotoInst->getTarget()->getIRName() << ")";
                }
                std::cout << std::endl;
                shouldKeep = false;
                optimized = true;
            }
        }
        */
        // 检查是否是return后的死代码（只对return语句后的代码进行死代码消除）
        if (i > 0 && isReturnInstruction(instructions[i - 1]) && !isLabelInstruction(inst)) {
            // 前一条指令是return指令，当前指令不是标签，则是死代码
            shouldKeep = false;
            optimized = true;
            std::cout << "Removing dead instruction after return at position " << i << std::endl;
        }

        if (shouldKeep) {
            newInstructions.push_back(inst);
        }
    }

    instructions = newInstructions;
    return optimized;
}

/// @brief 检查跳转是否是冗余的（跳转到下一条指令）
/// @param instructions 指令序列
/// @param index 跳转指令的索引
/// @return 是否是冗余跳转
bool DeadCodeElimination::isRedundantJump(const std::vector<Instruction *> & instructions, int index)
{
    if (index >= instructions.size() - 1) {
        return false; // 最后一条指令，不是冗余跳转
    }

    Instruction * jumpInst = instructions[index];
    if (jumpInst->getOp() != IRInstOperator::IRINST_OP_GOTO) {
        return false;
    }

    GotoInstruction * gotoInst = dynamic_cast<GotoInstruction *>(jumpInst);
    if (!gotoInst) {
        return false;
    }

    LabelInstruction * target = gotoInst->getTarget();
    if (!target) {
        return false;
    }

    // 检查下一条指令是否就是目标标签
    for (int i = index + 1; i < instructions.size(); ++i) {
        Instruction * nextInst = instructions[i];

        // 如果遇到标签指令
        if (isLabelInstruction(nextInst)) {
            LabelInstruction * nextLabel = dynamic_cast<LabelInstruction *>(nextInst);
            if (nextLabel == target) {
                return true; // 跳转到紧接着的标签，是冗余的
            }
            break; // 遇到其他标签，不是冗余跳转
        }

        // 如果遇到非标签指令，说明下一条指令不是标签
        break;
    }

    return false;
}

/// @brief 检查指令是否是终结指令（跳转、分支、返回）
/// @param inst 指令
/// @return 是否是终结指令
bool DeadCodeElimination::isTerminatorInstruction(Instruction * inst)
{
    if (!inst) {
        return false;
    }

    IRInstOperator op = inst->getOp();
    return (op == IRInstOperator::IRINST_OP_GOTO ||   // 无条件跳转
            op == IRInstOperator::IRINST_OP_BRANCH || // 条件跳转
            op == IRInstOperator::IRINST_OP_RET);     // 返回
}

/// @brief 检查指令是否是return指令
/// @param inst 指令
/// @return 是否是return指令
bool DeadCodeElimination::isReturnInstruction(Instruction * inst)
{
    if (!inst) {
        return false;
    }

    return inst->getOp() == IRInstOperator::IRINST_OP_RET;
}

/// @brief 检查指令是否是标签指令
/// @param inst 指令
/// @return 是否是标签指令
bool DeadCodeElimination::isLabelInstruction(Instruction * inst)
{
    if (!inst) {
        return false;
    }

    return inst->getOp() == IRInstOperator::IRINST_OP_LABEL;
}

/// @brief 获取跳转指令的目标标签
/// @param inst 跳转指令
/// @return 目标标签，如果不是跳转指令则返回nullptr
LabelInstruction * DeadCodeElimination::getJumpTarget(Instruction * inst)
{
    if (!inst || inst->getOp() != IRInstOperator::IRINST_OP_GOTO) {
        return nullptr;
    }

    GotoInstruction * gotoInst = dynamic_cast<GotoInstruction *>(inst);
    return gotoInst ? gotoInst->getTarget() : nullptr;
}

/// @brief 获取分支指令的目标标签
/// @param inst 分支指令
/// @param getTrueTarget 是否获取真分支目标（否则获取假分支目标）
/// @return 目标标签，如果不是分支指令则返回nullptr
LabelInstruction * DeadCodeElimination::getBranchTarget(Instruction * inst, bool getTrueTarget)
{
    if (!inst || inst->getOp() != IRInstOperator::IRINST_OP_BRANCH) {
        return nullptr;
    }

    BranchInstruction * branchInst = dynamic_cast<BranchInstruction *>(inst);
    if (!branchInst) {
        return nullptr;
    }

    return getTrueTarget ? branchInst->getTrueLabel() : branchInst->getFalseLabel();
}

/// @brief 查找标签指令在指令序列中的位置
/// @param instructions 指令序列
/// @param label 要查找的标签
/// @return 标签在序列中的索引，如果未找到则返回-1
int DeadCodeElimination::findLabelIndex(const std::vector<Instruction *> & instructions, LabelInstruction * label)
{
    for (int i = 0; i < instructions.size(); ++i) {
        if (instructions[i] == label) {
            return i;
        }
    }
    return -1;
}

/// @brief 构建控制流图
/// @param instructions 指令序列
/// @return 基本块列表
std::vector<DeadCodeElimination::BasicBlock *>
DeadCodeElimination::buildControlFlowGraph(const std::vector<Instruction *> & instructions)
{
    std::vector<BasicBlock *> blocks;
    if (instructions.empty()) {
        return blocks;
    }

    // 第一步：识别基本块的边界
    std::set<int> blockStarts;
    blockStarts.insert(0); // 第一条指令总是基本块的开始

    // 找到所有标签指令和跳转目标
    for (int i = 0; i < instructions.size(); ++i) {
        Instruction * inst = instructions[i];

        // 标签指令是基本块的开始
        if (isLabelInstruction(inst)) {
            blockStarts.insert(i);
        }

        // 终结指令后的指令是基本块的开始
        if (isTerminatorInstruction(inst) && i + 1 < instructions.size()) {
            blockStarts.insert(i + 1);
        }
    }

    // 第二步：创建基本块
    std::vector<int> starts(blockStarts.begin(), blockStarts.end());
    for (int i = 0; i < starts.size(); ++i) {
        BasicBlock * block = new BasicBlock();

        int start = starts[i];
        int end = (i + 1 < starts.size()) ? starts[i + 1] : instructions.size();

        // 添加指令到基本块
        for (int j = start; j < end; ++j) {
            block->instructions.push_back(instructions[j]);

            // 如果第一条指令是标签，记录它
            if (j == start && isLabelInstruction(instructions[j])) {
                block->label = dynamic_cast<LabelInstruction *>(instructions[j]);
            }
        }

        blocks.push_back(block);
    }

    // 第三步：建立基本块之间的连接关系
    for (int i = 0; i < blocks.size(); ++i) {
        BasicBlock * block = blocks[i];
        if (block->instructions.empty()) {
            continue;
        }

        Instruction * lastInst = block->instructions.back();

        if (lastInst->getOp() == IRInstOperator::IRINST_OP_GOTO) {
            // 无条件跳转
            LabelInstruction * target = getJumpTarget(lastInst);
            if (target) {
                for (BasicBlock * targetBlock: blocks) {
                    if (targetBlock->label == target) {
                        block->successors.insert(targetBlock);
                        targetBlock->predecessors.insert(block);
                        break;
                    }
                }
            }
        } else if (lastInst->getOp() == IRInstOperator::IRINST_OP_BRANCH) {
            // 条件跳转
            LabelInstruction * trueTarget = getBranchTarget(lastInst, true);
            LabelInstruction * falseTarget = getBranchTarget(lastInst, false);

            if (trueTarget) {
                for (BasicBlock * targetBlock: blocks) {
                    if (targetBlock->label == trueTarget) {
                        block->successors.insert(targetBlock);
                        targetBlock->predecessors.insert(block);
                        break;
                    }
                }
            }

            if (falseTarget) {
                for (BasicBlock * targetBlock: blocks) {
                    if (targetBlock->label == falseTarget) {
                        block->successors.insert(targetBlock);
                        targetBlock->predecessors.insert(block);
                        break;
                    }
                }
            }
        } else if (lastInst->getOp() != IRInstOperator::IRINST_OP_RET) {
            // 不是返回指令，则顺序执行到下一个基本块
            if (i + 1 < blocks.size()) {
                BasicBlock * nextBlock = blocks[i + 1];
                block->successors.insert(nextBlock);
                nextBlock->predecessors.insert(block);
            }
        }
    }

    return blocks;
}

/// @brief 标记可达的基本块
/// @param blocks 基本块列表
void DeadCodeElimination::markReachableBlocks(std::vector<BasicBlock *> & blocks)
{
    if (blocks.empty()) {
        return;
    }

    // 使用广度优先搜索标记可达的基本块
    std::queue<BasicBlock *> workList;

    // 第一个基本块总是可达的
    blocks[0]->reachable = true;
    workList.push(blocks[0]);

    while (!workList.empty()) {
        BasicBlock * current = workList.front();
        workList.pop();

        // 标记所有后继基本块为可达
        for (BasicBlock * successor: current->successors) {
            if (!successor->reachable) {
                successor->reachable = true;
                workList.push(successor);
            }
        }
    }
}

/// @brief 移除不可达的基本块
/// @param blocks 基本块列表
/// @param instructions 原始指令序列
/// @return 优化后的指令序列
std::vector<Instruction *> DeadCodeElimination::removeUnreachableBlocks(const std::vector<BasicBlock *> & blocks,
                                                                        const std::vector<Instruction *> & instructions)
{
    std::vector<Instruction *> newInstructions;

    for (const BasicBlock * block: blocks) {
        if (block->reachable) {
            // 添加可达基本块的所有指令
            for (Instruction * inst: block->instructions) {
                newInstructions.push_back(inst);
            }
        }
    }

    return newInstructions;
}

/// @brief 清理基本块内存
/// @param blocks 基本块列表
void DeadCodeElimination::cleanupBlocks(std::vector<BasicBlock *> & blocks)
{
    for (BasicBlock * block: blocks) {
        delete block;
    }
    blocks.clear();
}
