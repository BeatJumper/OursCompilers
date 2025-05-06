#include <queue>

#include "CFG.h"

/// @brief 活跃分析
/// @param _graph 需要被活跃分析的CFG图
void LiveVariableAnalysis(ControlFlowGraph * _graph);

/// @brief 集合合并的工具函数
/// @tparam T 集合内元素类型
/// @param a 集合a
/// @param b 集合b
/// @return 并入后集合a是否发生变化
/// @note 把集合b的元素全加入集合a
template <typename T>
bool merge_set(std::set<T> & a, std::set<T> & b);

/// @brief 集合求差集的工具函数
/// @tparam T 集合内元素类型
/// @param a 集合a
/// @param b 集合b
/// @return 作差后的集合
template <typename T>
std::set<T> set_difference(std::set<T> & a, std::set<T> & b);
