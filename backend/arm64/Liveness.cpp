#include "Liveness.h"
#include <iostream>

bool update_live(Node_Dataflow * node, Node_Dataflow * succ1, Node_Dataflow * succ2)
{
    printf("node:%lld\n", node);
    printset(node->liveIN);
    //  printset(node->liveOUT);
    // printset(node->def_set);
    // printset(node->use_set);
    //  代入数据流方程
    node->liveIN = set_difference(node->liveOUT, node->def_set);
    merge_set(node->liveIN, node->use_set);
    bool ret = false;
    if (succ1) {
        ret |= merge_set(node->liveOUT, succ1->liveIN);
    }
    if (succ2) {
        ret |= merge_set(node->liveOUT, succ2->liveIN);
    }
    return ret;
}

void LiveVariableAnalysis(ControlFlowGraph * _graph)
{
    auto & node_list = _graph->get_node_list();
    bool need_update = true;
    while (need_update) {

        // 当且仅当LIVEOUT在迭代后发生更改才继续迭代
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

            /*
            assert(node->getIRCode() != nullptr);
            printf("%d\n", node->getIRCode()->getCode().size());
            for (auto inst: node->getIRCode()->getCode()) {
                std::string s;
                inst->toString(s);
                std::cout << s << "\n";
            }
            assert(next_nodes[0]);
            */

            need_update |= update_live(dataflow_list[dataflow_list.size() - 1],
                                       // 第一个后继
                                       next_nodes[0] ? next_nodes[0]->dataflow_list[0] : nullptr,
                                       // 第二个后继（可能是nullptr)
                                       next_nodes[1] ? next_nodes[1]->dataflow_list[0] : nullptr);
        }
    }
}

/// @brief 集合合并的工具函数
/// @tparam T 集合内元素类型
/// @param a 集合a
/// @param b 集合b
/// @return 并入后集合a是否并入了新值
/// @note 把集合b的元素逐个加入集合a
/// @note 时间复杂度：O(Blog(A+B)) 其中A，B分别代表集合a和b的大小
template <typename T>
bool merge_set(std::set<T> & a, std::set<T> & b)
{
    int size0 = a.size();
    for (T element: b) {
        a.insert(element);
    }
    // std::cout << size0 << " " << a.size() << std::endl;
    return a.size() != size0;
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