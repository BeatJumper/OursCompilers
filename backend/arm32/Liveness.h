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
    /// @brief 干涉图节点对应的Value
    Value * val;
    /// @brief 干涉图节点的邻接表
    std::set<node_IG *> neighbors;
    /// @brief 构造函数
    /// @param _val 节点对应的Value
    node_IG(Value * _val);
    /// @brief 给干涉图节点添加邻居
    /// @param neighbor 邻居的指针
    void add_neighbor(node_IG * neighbor);
};
/// @brief 干涉图
struct InterferenceGraph {
    /// @brief 干涉图中的节点列表
    std::vector<node_IG *> node_list;
    /// @brief 干涉图中添加一条无向边
    /// @param node1 干涉图的一个节点
    /// @param node2 干涉图的另一个节点
    static void add_edge(node_IG * node1, node_IG * node2);
    /// @brief 对一个活跃分析后的控制流图创建干涉图
    /// @param graph 控制流图
    InterferenceGraph(ControlFlowGraph * graph);
    /// @brief 析构函数
    ~InterferenceGraph();
};