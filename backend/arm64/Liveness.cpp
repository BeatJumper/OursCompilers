#include "Liveness.h"
#include <iostream>

bool update_live(Node_Dataflow * node, Node_Dataflow * succ1, Node_Dataflow * succ2)
{
    bool ret = false;
    if (succ1) {
        ret |= merge_set(node->liveOUT, succ1->liveIN);
    }
    if (succ2) {
        ret |= merge_set(node->liveOUT, succ2->liveIN);
    }
    std::set<Value *> original_livein = node->liveIN;
    node->liveIN = set_difference(node->liveOUT, node->def_set);
    merge_set(node->liveIN, node->use_set);
    ret |= (original_livein != node->liveIN);

    /*
    std::string s;
    node->inst->toString(s);
    std::cout << "node:" << s << std::endl;
    std::cout << "size of LIVE_OUT:" << node->liveOUT.size() << std::endl;
    printset(node->liveOUT);
    std::cout << "size of LIVE_IN:" << node->liveIN.size() << std::endl;
    printset(node->liveIN);
    std::cout << "size of def_set:" << node->def_set.size() << std::endl;
    printset(node->def_set);
    std::cout << "size of use_set:" << node->use_set.size() << std::endl;
    printset(node->use_set);
    */

    return ret;
}

void LiveVariableAnalysis(ControlFlowGraph * _graph)
{
    auto & node_list = _graph->get_node_list();
    bool need_update = true;
    // bool first_time = true;
    int i = 1;
    while (need_update) {
        printf("第%d轮迭代\n", i++);
        need_update = false;
        /*
        // 除非是第一轮迭代，否则当且仅当LIVEOUT在迭代后发生更改才继续迭代
        if (first_time) {
            // 在第一次更新时只有LIVEIN发生变化，所以先让它再多跑一次试试
            first_time = false;
        } else {
            need_update = false;
        }
        */
        for (Node_CFG * node: node_list) {
            auto & dataflow_list = node->get_dataflow_list();
            // 基本块的末端（跳转指令）可能有多个后继
            Node_CFG ** next_nodes = node->get_next_nodes();
            need_update |= update_live(dataflow_list[dataflow_list.size() - 1],
                                       // 第一个后继
                                       next_nodes[0] ? next_nodes[0]->dataflow_list[0] : nullptr,
                                       // 第二个后继（可能是nullptr)
                                       next_nodes[1] ? next_nodes[1]->dataflow_list[0] : nullptr);
            for (int i = dataflow_list.size() - 2; i >= 0; --i) {
                std::string s;
                dataflow_list[i]->inst->toString(s);
                std::cout << "node:" << s << std::endl;
                std::cout << "size of def_set:" << dataflow_list[i]->def_set.size() << std::endl;
                // printset(dataflow_list[i]->def_set);
                std::cout << "size of use_set:" << dataflow_list[i]->use_set.size() << std::endl;
                // printset(dataflow_list[i]->use_set);
                //  对于基本块内的前 n-1 个指令，只会有1个后继指令
                need_update |= update_live(dataflow_list[i], dataflow_list[i + 1]);
            }
        }
    }
}

/// @brief 集合求差集的工具函数
/// @tparam T 集合内元素类型
/// @param a 集合a
/// @param b 集合b
/// @return 作差后的集合
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