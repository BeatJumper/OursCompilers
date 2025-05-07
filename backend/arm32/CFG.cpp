#include "GotoInstruction.h"
#include "Instruction.h"
#include "Liveness.h"
#include <cassert>

ControlFlowGraph::ControlFlowGraph(Function * func)
{
    // TODO 此处缺少所需基本块的实现
    InterCode BasicIRBlock;
    for (BasicIRBlock) {
        Node_CFG * node = new Node_CFG(this, BasicIRBlock);
        node_list.push_back(node);
    }
    for (Node_CFG * node: node_list) {
        for (std::string label: node->get_son_label_list()) {
            Node_CFG * son = get_CFG_from_label(label);
            if (son) {
                node->add_successor(get_CFG_from_label(label));
            }
        }
    }
}

Node_CFG::Node_CFG(ControlFlowGraph * _graph, InterCode & BasicIRBlock)
{
    for (Instruction * inst: (BasicIRBlock.getCode())) {
        // TODO 还需要添加条件跳转指令

        // 添加语句对应的数据流节点
        dataflow_list.push_back(new Node_Dataflow(inst));
        switch (inst->getOp()) {
            case IRInstOperator::IRINST_OP_GOTO: {
                // 无条件跳转指令的目标Label名
                std::string target_label = ((GotoInstruction *) inst)->getTarget()->getIRName();
                // 记录子节点Label名
                add_label_for_successor(target_label);
                break;
            }
            case IRInstOperator::IRINST_OP_LABEL:
                //检测控制流的Label并贴上
                std::string label = ((LabelInstruction *) inst)->getIRName();
                _graph->add_label_for_CFG(label, this);
                break;
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

Node_CFG * ControlFlowGraph::get_CFG_from_label(std::string label)
{
    if (LabelToNodeCFG.find(label) != LabelToNodeCFG.end()) {
        return LabelToNodeCFG[label];
    }
    return nullptr;
}

void Node_CFG::add_label_for_successor(std::string label)
{
    son_labels.insert(label);
}

std::set<std::string> & Node_CFG::get_son_label_list()
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