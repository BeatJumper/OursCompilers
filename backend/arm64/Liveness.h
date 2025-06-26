#ifndef Liveness_H
#define Liveness_H
#include "CFG.h"

// 定义宏来遍历 set 中的二元组合
#define FOR_EACH_PAIR_IN_SET(set)                                                                                      \
    for (auto it1 = (set).begin(); it1 != (set).end(); ++it1)                                                          \
        for (auto it2 = std::next(it1); it2 != (set).end(); ++it2)

/// @brief 对一个数据流节点更新LIVEIN和LIVEOUT
/// @param node 要更新的数据流节点
/// @param succ1 数据流节点的后继
/// @param succ2 数据流节点的另一后继（如果存在），不存在则缺省
/// @return LIVEOUT是否发生更改
bool update_live(Node_Dataflow * node, Node_Dataflow * succ1, Node_Dataflow * succ2 = nullptr);

/// @brief 活跃分析
/// @param _graph 需要被活跃分析的CFG图
void LiveVariableAnalysis(ControlFlowGraph * _graph);

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
    return a.size() != size0;
}

/// @brief 集合求差集的工具函数
/// @tparam T 集合内元素类型
/// @param a 集合a
/// @param b 集合b
/// @return 作差后的集合
template <typename T>
std::set<T> set_difference(std::set<T> & a, std::set<T> & b);

#endif