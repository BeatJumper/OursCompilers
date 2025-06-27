#include "GotoInstruction.h"
#include "BranchInstruction.h"
#include "Instruction.h"
#include "Liveness.h"
#include <cassert>
#include "AllocaInstruction.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
#include "MoveInstruction.h"

ControlFlowGraph::ControlFlowGraph(Function * func)
{
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
            // std::cout << node->getIRCode()->getCode().size() << std::endl;
            node->get_next_nodes()[i++] = son;
            assert(node->get_next_nodes()[0]);
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

InterCode * Node_CFG::getIRCode()
{
    return IRCode;
}

Node_CFG ** Node_CFG::get_next_nodes()
{
    return next_nodes;
}

Node_Dataflow::Node_Dataflow(Instruction * _inst) : inst(_inst)
{
    /*
    // 如果是void的返回值，则返回值不占用寄存器
    if (inst->hasResultValue()) {
        def_set.insert(inst);
    }

    for (auto usee: inst->getOperandsValue()) {
        if (def_set.count(usee) == 0) {
            // use集中不能包含刚刚def的元素
            use_set.insert(usee);
        }
    }
    */
    /*
     * 这里添加一系列指令类型检测的原因：
     * 有的指令把不产生数据流的变量也存进了操作数，
     * 同时有的指令的返回值不是他自己的Instruction*
     * ......由于各种原因所以需要特判。
     */
    // printf("产生数据流节点\n");
    if (Instanceof(inst, AllocaInstruction *, _inst)) {
        // Alloc指令没有直接数据流，所以不做任何事
    } else if (Instanceof(inst, StoreInstruction *, _inst)) {
        use_set.insert(inst->getOperand(0));
    } else if (Instanceof(inst, LoadInstruction *, _inst)) {
        def_set.insert(inst);
    } else if (Instanceof(inst, MoveInstruction *, _inst)) {
        def_set.insert(inst->getOperand(0));
        Value * source = inst->getOperand(1);
        if (Instanceof(constvar, Constant *, source)) {
            // 什么都不做
        } else {
            use_set.insert(source);
        }
    } else if (Instanceof(inst, Instruction *, _inst)) {
        // printf("其它指令\n");
        //  Instanceof(inst, Instruction *, _inst);
        if (inst->hasResultValue()) {
            def_set.insert(inst);
        }

        for (auto usee: inst->getOperandsValue()) {
            // 除了store指令以外的立即数都不需要寄存器
            if (Instanceof(constusee, Constant *, usee) == nullptr && def_set.count(usee) == 0) {
                // use集中不能包含刚刚def的元素
                use_set.insert(usee);
            }
        }
    }
}

std::vector<Node_Dataflow *> & Node_CFG::get_dataflow_list()
{
    return dataflow_list;
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
        // std::cout << s << "\n";
    }

    /*
    auto insts = BasicIRBlock->getInsts();
    // 遍历，寻找溢出变量并处理
    for (int i = 0; i < insts.size();) {
        Instruction * inst = insts[i];

        // 对溢出变量的每次USE，都用一个新Value代替，这个新Value即为StackLdrInstruction的返回值Value
        for (int op_index = 0; op_index < inst->getOperandsNum(); op_index++) {
            Value * op_val = inst->getOperand(op_index);
            if (op_val->get_isleaked()) {
                // 新的Value
                StackLdrInstruction * newval = new StackLdrInstruction(_graph->get_func(), op_val, op_val->getType());
                // 插入新Value的取内存指令
                insts.insert(insts.begin() + i, newval);
                // 是在i位置前面插入的，所以插入后i位置是新插入的Value，需要把i额外加1
                i++;
                // 溢出变量的出现也替换为新Value了
                inst->getOperands()[op_index]->setUsee(newval);
            }
        }
        i++;
    }
    */

    for (Instruction * inst: (BasicIRBlock->getInsts())) {
        // 添加语句对应的数据流节点
        dataflow_list.push_back(new Node_Dataflow(inst));

        // get_value_list()方法弃用
        /*
        // 记录到Value表
        _graph->get_value_list().insert(inst);
        */

        switch (inst->getOp()) {
            case IRInstOperator::IRINST_OP_GOTO: {
                // 无条件跳转指令的目标Label名
                LabelInstruction * target_label = ((GotoInstruction *) inst)->getTarget();
                // 记录子节点Label名
                add_label_for_successor(target_label);
                // printf("有Goto\n");
                break;
            }
            case IRInstOperator::IRINST_OP_LABEL: {
                // 将基本块自己的Label指令和自己的控制流节点联系起来
                _graph->add_label_for_CFG((LabelInstruction *) inst, this);
                // printf("有Label\n");
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