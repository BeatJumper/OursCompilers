#include "Liveness.h"
#include <iostream>

bool update_live(Instruction * node, Instruction * succ1, Instruction * succ2)
{
    bool ret = false;
    if (succ1) {
        ret |= merge_set(node->get_liveout(), succ1->get_livein());
    }
    if (succ2) {
        ret |= merge_set(node->get_liveout(), succ2->get_livein());
    }
    int original_livein_size = node->get_livein().size();
    merge_set(node->get_livein(), node->get_liveout());
    set_difference(node->get_livein(), node->get_def_set());
    ret |= (original_livein_size != node->get_livein().size());

    return ret;
}

void LiveVariableAnalysis(ControlFlowGraph * _graph)
{
    auto & node_list = _graph->get_node_list();
    bool need_update = true;

    while (need_update) {
        need_update = false;
        for (Node_CFG * node: node_list) {
            // 基本块的末端（跳转指令）可能有多个后继
            Node_CFG ** next_nodes = node->get_next_nodes();
            auto & insts = node->getIRCode()->getCode();
            need_update |= update_live(insts[insts.size() - 1],
                                       // 第一个后继
                                       next_nodes[0] ? next_nodes[0]->getIRCode()->getCode()[0] : nullptr,
                                       // 第二个后继（可能是nullptr)
                                       next_nodes[1] ? next_nodes[1]->getIRCode()->getCode()[0] : nullptr);
            for (int i = insts.size() - 2; i >= 0; --i) {
                //   对于基本块内的前 n-1 个指令，只会有1个后继指令
                need_update |= update_live(insts[i], insts[i + 1]);
            }
        }
    }
}

/// @brief 集合求差集的工具函数
/// @tparam T 集合内元素类型
/// @param a 集合a
/// @param b 集合b
/// @note 从a中删去b拥有的元素
template <typename T>
static void set_difference(std::set<T> & a, std::set<T> & b)
{
    for (T element: b) {
        a.erase(element);
    }
}