#include "CFG.h"
#include "GotoInstruction.h"
#include "Instruction.h"
#include "ILocArm32.h"

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
        // TODO 还需要添加条件跳转指令（前端还没有条件跳转指令的实现）
        switch (inst->getOp()) {
            case IRInstOperator::IRINST_OP_GOTO:
                // 无条件跳转指令的目标Label名
                std::string target_label = ((GotoInstruction *) inst)->getTarget()->getIRName();
                // 记录子节点Label名
                add_label_for_successor(target_label);
                break;
            case IRInstOperator::IRINST_OP_LABEL:
                //检测控制流的Label并贴上
                std::string label = ((LabelInstruction *) inst)->getIRName();
                _graph->add_label_for_CFG(label, this);
                break;
            default:
                //对于其它节点，相当于发生了一次计算，所以统计计算的def和use
                def_set.insert(inst);
                for (auto usee: inst->getOperandsValue()) {
                    if (def_set.count(usee) == 0) {
                        // use集中不能包含刚刚def的元素
                        use_set.insert(usee);
                    }
                }
                break;
        }
    }
}

void Node_CFG::add_successor(Node_CFG * successor)
{
    next_nodes.insert(successor);
}

bool ControlFlowGraph::add_label_for_CFG(std::string label, Node_CFG * node)
{
    if (LabelToNodeCFG.find(label) != LabelToNodeCFG.end()) {
        // 如果该Label已经被使用则粘贴失败
        return false;
    }
    LabelToNodeCFG[label] = node;
    return true;
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