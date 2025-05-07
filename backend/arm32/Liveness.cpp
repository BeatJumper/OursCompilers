#include "Liveness.h"

bool update_live(Node_Dataflow * node, Node_Dataflow * succ1, Node_Dataflow * succ2)
{
    // 代入数据流方程
    node->liveIN = set_difference(node->liveOUT, node->def_set);
    merge_set(node->liveIN, node->use_set);
    bool ret = false;
    ret |= merge_set(node->liveOUT, succ1->liveIN);
    if (succ2) {
        ret |= merge_set(node->liveOUT, succ2->liveIN);
    }
}
void LiveVariableAnalysis(ControlFlowGraph * _graph)
{
    auto & node_list = _graph->get_node_list();
    bool need_update = true;
    while (need_update) {
        need_update = false;
        for (Node_CFG * node: node_list) {
            auto & dataflow_list = node->get_dataflow_list();

            // 先更新前 n-1 个节点的LIVE值
            for (int i = 0; i < dataflow_list.size() - 1; i++) {

                // 对于基本块内的前 n-1 个指令，只会有1个后继指令
                need_update |= update_live(dataflow_list[i], dataflow_list[i + 1]);
            }

            // 基本块的末端（跳转指令）可能有多个后继
            Node_CFG ** next_nodes = node->get_next_nodes();
            need_update |= update_live(dataflow_list[dataflow_list.size() - 1],
                                       // 第一个后继
                                       next_nodes[0]->dataflow_list[0],
                                       // 第二个后继（可能是nullptr)
                                       next_nodes[1] ? next_nodes[1]->dataflow_list[0] : nullptr);
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

node_IG::node_IG(Value * _val) : val(_val)
{}
InterferenceGraph::InterferenceGraph(Node_CFG * node_cfg)
{
    for (Value * val: node_cfg->get_value_list()) {
        node_list.push_back(new node_IG(val));
    }
}