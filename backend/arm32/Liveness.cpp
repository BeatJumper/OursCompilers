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
            need_update |= update_live(dataflow_list[dataflow_list.size() - 1],
                                       // 第一个后继
                                       next_nodes[0]->dataflow_list[0],
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
    return a.size() == size0;
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

node_IG::node_IG(Value * _val) : val(_val)
{}

void node_IG::add_neighbor(node_IG * neighbor)
{
    neighbors.insert(neighbor);
}

void InterferenceGraph::add_edge(node_IG * node1, node_IG * node2)
{
    node1->add_neighbor(node2);
    node2->add_neighbor(node1);
}

InterferenceGraph::InterferenceGraph(ControlFlowGraph * graph)
{

    // 从Value到干涉图节点的映射
    std::map<Value *, node_IG *> value_to_ig;

    for (Value * val: graph->get_value_list()) {
        node_IG * newnode = new node_IG(val);
        value_to_ig[val] = newnode;
        node_list.push_back(newnode);
    }

    //扫描函数里每条指令，获取每个时刻的活跃变量集合
    for (Node_CFG * node_cfg: graph->get_node_list()) {
        for (Node_Dataflow * node_data: node_cfg->get_dataflow_list()) {

            // 某个指令位置下活跃着的量的集合
            std::set<Value *> value_occupy = node_data->liveOUT;
            merge_set(value_occupy, node_data->def_set);

            // 这些不同的量两两之间都是互斥的，不能在同一寄存器
            FOR_EACH_PAIR_IN_SET(value_occupy)
            {
                // 在干涉图中连上一条边
                add_edge(value_to_ig[*it1], value_to_ig[*it2]);
            }
        }
    }
}