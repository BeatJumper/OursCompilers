#ifndef InterferenceGraph_H
#define InterferenceGraph_H

#include "Liveness.h"

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
    /// @brief 删除一个邻居的信息
    /// @param neighbor 要删除的邻居
    void remove_neighbor(node_IG * neighbor);
    /// @brief 获得节点的度数
    /// @return 节点度数
    int degree();
    /// @brief 目前的颜色（-1表示还没涂色）
    int color = -1;
};
/// @brief 干涉图
struct InterferenceGraph {
    /// @brief 本干涉图所属的控制流图
    ControlFlowGraph * graph_cfg;
    /// @brief 干涉图中的节点列表
    std::set<node_IG *> node_set;
    /// @brief 没有被预染色的节点的列表
    std::set<node_IG *> uncolored_node_set;
    /// @brief 干涉图中添加一条无向边
    /// @param node1 干涉图的一个节点
    /// @param node2 干涉图的另一个节点
    void add_edge(node_IG * node1, node_IG * node2);
    /// @brief 将一个节点从图中删除（可恢复）
    /// @param node 要删除的节点
    void remove_node(node_IG * node);
    /// @brief 恢复 remove_node 删除的节点
    /// @param node 要恢复的节点
    void restore_node(node_IG * node);
    /// @brief 擦除图中所有颜色
    void flush_all_color();
    /// @brief 基本块划分
    /// @param func 要处理的函数
    void GenBasicBlocks(Function * func);
    /// @brief 由一个函数创建干涉图
    /// @param func 需要创建干涉图的函数
    InterferenceGraph(Function * func);
    /// @brief 对一个活跃分析后的控制流图创建干涉图
    /// @param graph 控制流图
    void ExecuteCFG(ControlFlowGraph * graph);
    /// @brief 析构函数
    ~InterferenceGraph();
    /// @brief 对一个干涉图染色
    /// @param graph 干涉图
    /// @param color_size 颜色种类
    /// @return 是否染色成功
    static bool color_graph(InterferenceGraph * graph, int color_size);
    /// @brief 现有的主要染色算法是 Welsh-Powell算法、回溯法
    enum class color_method { WELSH_POWELL, BACKTRACK };
    /// @brief 目前选择的染色算法（默认是WELSH_POWELL算法）
    static const color_method method_chosen = color_method::WELSH_POWELL;
    /// @brief 将染的颜色对应到对应的寄存器号码
    /// @param color 颜色
    /// @return 对应的寄存器号码
    /// @note 染色中可使用的寄存器目前有X0-X15以及X19-X28，一共26个
    static int ColorToRegId(int color);
};

/// @brief 对一个干涉图节点，寻找其目前能染的编号最小的颜色
/// @param node 干涉图节点
/// @param color_size 所有颜色总个数（假设颜色编号1~color_size)
/// @return 寻找到的最小颜色（找不到则返回-1）
static int least_color_for_node(node_IG * node, int color_size);

/// @brief Welsh-Powell算法
/// @param graph 要染色的图
/// @param color_size 可用颜色个数
/// @return 染色是否成功
static bool welsh_powell(InterferenceGraph * graph, int color_size);

/// @brief 回溯法染色过程
/// @param graph 要染色的图
/// @param color_size 可用颜色个数
/// @param iter 目前考虑的节点的迭代器
/// @return 染色是否成功
static bool backtrack_color(InterferenceGraph * graph, int color_size, std::set<node_IG *>::iterator iter);

#endif