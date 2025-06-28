#ifndef CFG_H
#define CFG_H
#pragma once
#include <vector>
#include <set>
#include <map>
#include <iostream>

#include "Function.h"
#include "LabelInstruction.h"
#include "CodeGeneratorArm64.h"

class ControlFlowGraph;

static void printval(Value * x)
{
    std::cout << x->getIRName() << std::endl;
    if (Instanceof(y, Instruction *, x)) {
        std::string s;
        y->toString(s);
        std::cout << s << std::endl;
    }
}
static void printset(std::set<Value *> & S)
{
    for (auto x: S) {
        printval(x);
    }
    // std::cout << std::endl;
}
/// @brief 控制流图中的节点
class Node_CFG {
public:
    /// @brief 构造函数
    /// @param _graph 所属控制流图
    /// @param BasicIRBlock 基本块
    Node_CFG(ControlFlowGraph * _graph, InterCode * BasicIRBlock);

    /// @brief 析构函数
    ~Node_CFG();

    /// @brief 添加后继节点
    /// @param successor 后继节点的指针
    void add_successor(Node_CFG * successor);

    /// @brief 添加一个控制流块的子节点的Label
    /// @param successor Label指令
    void add_label_for_successor(LabelInstruction * label);

    /// @brief 获取节点内会跳转到的Label列表
    /// @return 所求Label列表
    std::set<LabelInstruction *> & get_son_label_list();

    /// @brief 获取子节点列表
    /// @return 基本块的子节点列表
    Node_CFG ** get_next_nodes();

    /// @brief 对应IR代码块的getter
    /// @return 对应IR代码块
    InterCode * getIRCode();

    // 定义友元函数，使其直接能访问private
    friend void LiveVariableAnalysis(ControlFlowGraph * _graph);

private:
    /// @brief 一个基本块最多有2个后继
    static const int MAX_NUM_OF_SON = 2;
    /// @brief 节点自己的邻接表
    Node_CFG * next_nodes[MAX_NUM_OF_SON] = {nullptr, nullptr};
    /**
     * @brief  控制流节点的直接后继节点的Label的集合
     * @note 在通过跳转指令寻找控制流图中的后继控制流块时，控制流图中有的Label对应的控制流块可能还没初始化，
     * 此时无法正确把Label转为控制流块，只能用这个集合把Label暂存下来，等到所有块初始化好后再进行Label转换。
     */
    std::set<LabelInstruction *> son_labels;
    /// @brief 包含的IR代码块
    InterCode * IRCode;
};

/// @brief 控制流图
class ControlFlowGraph {
public:
    /// @brief 构造函数
    /// @param func 需要建立控制流图的函数
    ControlFlowGraph(Function * func);

    /// @brief 析构函数
    ~ControlFlowGraph();

    /// @brief 把一个Label指令与对应的控制流块的对应关系记录下来
    /// @param label 控制流块对应的Label指令
    /// @param node 控制流块的指针
    /// @return 是否成功
    bool add_label_for_CFG(LabelInstruction * label, Node_CFG * node);

    /// @brief 获得Label对应的控制流块
    /// @param label 要查找的Label
    /// @return 控制流块，若找不到则报错
    Node_CFG * get_CFG_from_label(LabelInstruction * label);

    /// @brief 获取CFG的节点
    /// @return 节点表
    std::vector<Node_CFG *> & get_node_list();

    /*
    /// @brief Value表的getter
    /// @return Value表
    std::set<Value *> & get_value_list();
    */

    /// @brief 所属函数的getter
    /// @return 所属函数的Function类指针
    Function * get_func();

private:
    /// @brief Label到控制流节点的映射表
    std::map<LabelInstruction *, Node_CFG *> LabelToNodeCFG;
    /// @brief 控制流图中的节点列表
    std::vector<Node_CFG *> node_list;
    /// @brief 控制流图中出现过的所有Value
    std::set<Value *> value_list;
    /// @brief 所属函数
    Function * func;
};

#endif