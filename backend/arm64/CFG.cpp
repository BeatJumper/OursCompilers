#include "GotoInstruction.h"
#include "BranchInstruction.h"
#include "Instruction.h"
#include "Liveness.h"
#include <cassert>

ControlFlowGraph::ControlFlowGraph(Function * func)
{
    // 对基本块表里每个基本块都创建一个新的控制流节点
    for (InterCode * BasicIRBlock: func->getBasicBlocks()) {
        Node_CFG * node = new Node_CFG(this, *BasicIRBlock);
        node_list.push_back(node);
    }

    for (Node_CFG * node: node_list) {
        for (LabelInstruction * label: node->get_son_label_list()) {
            Node_CFG * son = get_CFG_from_label(label);
            assert(son != nullptr);
        }
    }
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

Node_CFG ** Node_CFG::get_next_nodes()
{
    return next_nodes;
}

Node_Dataflow::Node_Dataflow(Instruction * _inst) : inst(_inst)
{
    def_set.insert(inst);
    for (auto usee: inst->getOperandsValue()) {
        if (def_set.count(usee) == 0) {
            // use集中不能包含刚刚def的元素
            use_set.insert(usee);
        }
    }
}

std::vector<Node_Dataflow *> & Node_CFG::get_dataflow_list()
{
    return dataflow_list;
}

std::set<Value *> & ControlFlowGraph::get_value_list()
{
    return value_list;
}

bool ControlFlowGraph::add_label_for_CFG(LabelInstruction * label, Node_CFG * node)
{
    if (LabelToNodeCFG.count(label)) {
        return false;
    }
    LabelToNodeCFG[label] = node;
    return true;
}

Node_CFG::Node_CFG(ControlFlowGraph * _graph, InterCode & BasicIRBlock)
{
    for (Instruction * inst: (BasicIRBlock.getCode())) {
        // 添加语句对应的数据流节点
        dataflow_list.push_back(new Node_Dataflow(inst));
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