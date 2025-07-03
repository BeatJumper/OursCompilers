#include "Liveness.h"
#include <iostream>

bool update_live(Instruction * node, Instruction * succ1, Instruction * succ2)
{
    /*
    printf("活跃分析\n");
    printval(node);
    printf("NUM OF OPERANDS:%d\n", node->getOperandsNum());
    printf("DEF:\n");
    printset(node->get_def_set());
    printf("USE:\n");
    printset(node->get_use_set());
    printf("LIVEOUT:\n");
    printset(node->get_liveout());
    printf("LIVEIN\n");
    printset(node->get_livein());
    printf("\n");
    */
    bool ret = false;
    if (succ1) {
        /*
        printf("SUCC1\n");
        printval(succ1);
        printf("改动前LIVEOUT\n");
        printset(node->get_liveout());
        */
        // assert(node->get_liveout().size() < 2);
        ret |= merge_set(node->get_liveout(), succ1->get_livein());
        /*
        printf("改动后LIVEOUT\n");
        printset(node->get_liveout());
        */
        // assert(node->get_liveout().size() < 2);
    }
    if (succ2) {
        /*
        printf("SUCC2\n");
        printval(succ2);
        */
        ret |= merge_set(node->get_liveout(), succ2->get_livein());
    }
    int original_livein_size = node->get_livein().size();
    merge_set(node->get_livein(), node->get_liveout());
    set_difference(node->get_livein(), node->get_def_set());
    ret |= (original_livein_size != node->get_livein().size());

    /*
    std::string s;
    node->inst->toString(s);
    std::cout << "node:" << s << std::endl;
    std::cout << "size of LIVE_OUT:" << node->get_liveout().size() << std::endl;
    printset(node->get_liveout());
    std::cout << "size of LIVE_IN:" << node->get_livein().size() << std::endl;
    printset(node->get_livein());
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
    // int i = 1;
    while (need_update) {
        // printf("第%d轮迭代\n", i++);
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
            // 基本块的末端（跳转指令）可能有多个后继
            Node_CFG ** next_nodes = node->get_next_nodes();
            auto & insts = node->getIRCode()->getCode();
            // printf("%d\n", insts.size());
            need_update |= update_live(insts[insts.size() - 1],
                                       // 第一个后继
                                       next_nodes[0] ? next_nodes[0]->getIRCode()->getCode()[0] : nullptr,
                                       // 第二个后继（可能是nullptr)
                                       next_nodes[1] ? next_nodes[1]->getIRCode()->getCode()[0] : nullptr);
            for (int i = insts.size() - 2; i >= 0; --i) {
                // std::string s;
                // insts[i]->toString(s);
                //   对于基本块内的前 n-1 个指令，只会有1个后继指令
                need_update |= update_live(insts[i], insts[i + 1]);
                // printf("%d\n", i);
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