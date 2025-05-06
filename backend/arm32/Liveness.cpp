#include "Liveness.h"

void LiveVariableAnalysis(ControlFlowGraph * _graph)
{
    auto & node_list = _graph->get_node_list();
    bool need_update = true;
    while (need_update) {
        for (Node_CFG * node: node_list) {
            //代入数据流方程
            node->liveIN = set_difference(node->liveOUT, node->def_set);
            merge_set(node->liveIN, node->use_set);
            for (Node_CFG * succ: node->get_next_nodes()) {
                // 当两轮的liveOUT完全一样时，说明出现循环，不再有更新
                need_update |= merge_set(node->liveOUT, succ->liveIN);
            }
        }
    }
}

template <typename T>
bool merge_set(std::set<T> & a, std::set<T> & b)
{
    int size0 = a.size();
    for (T element: b) {
        a.insert(element);
    }
    return a.size() == size0;
}

template <typename T>
std::set<T> set_difference(std::set<T> & a, std::set<T> & b)
{
    std::set<T> ret;
    for (T element: a) {
        if (b.count(element) == 0) {
            ret.insert(element);
        }
    }
    return ret;
}