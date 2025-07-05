#include "GotoInstruction.h"
#include "BranchInstruction.h"
#include "Instruction.h"
#include "Liveness.h"
#include <cassert>
#include "AllocaInstruction.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
#include "MoveInstruction.h"
#include "FuncCallInstruction.h"
#include "VoidType.h"

ControlFlowGraph::ControlFlowGraph(Function * func)
{
    // 记录所属函数
    this->func = func;

    // 对基本块表里每个基本块都创建一个新的控制流节点
    for (InterCode * BasicIRBlock: func->getBasicBlocks()) {
        Node_CFG * node = new Node_CFG(this, BasicIRBlock);
        node_list.push_back(node);
    }

    for (Node_CFG * node: node_list) {
        int i = 0;
        for (LabelInstruction * label: node->get_son_label_list()) {
            Node_CFG * son = get_CFG_from_label(label);
            assert(son != nullptr);

            node->get_next_nodes()[i++] = son;
            assert(node->get_next_nodes()[0]);
        }
    }
}

ControlFlowGraph::~ControlFlowGraph()
{
    // 清理所有CFG节点
    for (Node_CFG * node: node_list) {
        delete node;
    }
    node_list.clear();
    LabelToNodeCFG.clear();
}

Node_CFG::~Node_CFG()
{
    // 清理资源，但不删除IRCode，因为它属于Function管理
    son_labels.clear();
}

void Node_CFG::add_successor(Node_CFG * successor)
{
    if (next_nodes[0]) {
        next_nodes[1] = successor;
    } else {
        next_nodes[0] = successor;
    }
}

Node_CFG * ControlFlowGraph::get_CFG_from_label(LabelInstruction * label)
{
    assert(LabelToNodeCFG.find(label) != LabelToNodeCFG.end());
    return LabelToNodeCFG[label];
}

void Node_CFG::add_label_for_successor(LabelInstruction * label)
{
    son_labels.insert(label);
}

std::set<LabelInstruction *> & Node_CFG::get_son_label_list()
{
    return son_labels;
}

std::vector<Node_CFG *> & ControlFlowGraph::get_node_list()
{
    return node_list;
}

InterCode * Node_CFG::getIRCode()
{
    return IRCode;
}

Node_CFG ** Node_CFG::get_next_nodes()
{
    return next_nodes;
}

/*
std::set<Value *> & ControlFlowGraph::get_value_list()
{
    return value_list;
}
*/

bool ControlFlowGraph::add_label_for_CFG(LabelInstruction * label, Node_CFG * node)
{
    if (LabelToNodeCFG.count(label)) {
        return false;
    }
    LabelToNodeCFG[label] = node;
    return true;
}

Function * ControlFlowGraph::get_func()
{
    return func;
}

Node_CFG::Node_CFG(ControlFlowGraph * _graph, InterCode * BasicIRBlock)
{
    IRCode = BasicIRBlock;

    for (auto inst: IRCode->getCode()) {
        std::string s;
        inst->toString(s);

    }


    for (Instruction * inst: (BasicIRBlock->getInsts())) {
        // 计算DEF和USE
        inst->transfer();

        switch (inst->getOp()) {
            case IRInstOperator::IRINST_OP_GOTO: {
                // 无条件跳转指令的目标Label名
                LabelInstruction * target_label = ((GotoInstruction *) inst)->getTarget();
                // 记录子节点Label名
                add_label_for_successor(target_label);
                break;
            }
            case IRInstOperator::IRINST_OP_LABEL: {
                // 将基本块自己的Label指令和自己的控制流节点联系起来
                _graph->add_label_for_CFG((LabelInstruction *) inst, this);
                break;
            }
            case IRInstOperator::IRINST_OP_BRANCH: {
                // 一条分支指令会通往两个Label
                LabelInstruction *true_label = ((BranchInstruction *) inst)->getTrueLabel(),
                                 *false_label = ((BranchInstruction *) inst)->getFalseLabel();
                // 把它们记录下来
                add_label_for_successor(true_label);
                add_label_for_successor(false_label);
                break;
            }
            default:
                break;
        }
    }
}