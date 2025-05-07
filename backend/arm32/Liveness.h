#include <queue>

#include "CFG.h"

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

/// @brief 干涉图的节点
struct node_IG {
    /// @brief 干涉图节点对应的量
    Value * val;
    /// @brief 干涉图节点的邻接表
    std::set<node_IG *> next_nodes;
    /// @brief 构造函数
    /// @param _val 节点的值
    node_IG(Value * _val);
};
/// @brief 干涉图
struct InterferenceGraph {
    /// @brief 干涉图中的节点列表
    std::vector<node_IG *> node_list;
    /// @brief 对一个活跃分析后的基本块创建干涉图
    /// @param node_cfg 基本块在CFG中的节点
    InterferenceGraph(Node_CFG * node_cfg);
    /// @brief 析构函数
    ~InterferenceGraph();
};