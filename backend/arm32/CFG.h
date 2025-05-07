#include <vector>
#include <set>
#include <map>

#include "Function.h"

class Node_CFG;
class ControlFlowGraph;
class Node_Dataflow;

/// @brief 控制流图中的节点
class Node_CFG {
public:
    /// @brief 构造函数
    /// @param _graph 所属控制流图
    /// @param BasicIRBlock 基本块
    Node_CFG(ControlFlowGraph * _graph, InterCode & BasicIRBlock);

    /// @brief 析构函数
    ~Node_CFG();

    /// @brief 添加后继节点
    /// @param successor 后继节点的指针
    void add_successor(Node_CFG * successor);

    /// @brief 添加一个控制流块向外跳转到的Label
    /// @param successor Label名
    void add_label_for_successor(std::string label);

    /// @brief 获取节点内会跳转到的Label列表
    /// @return 所求Label列表
    std::set<std::string> & get_son_label_list();

    /// @brief 获取子节点列表
    /// @return 基本块的子节点列表
    Node_CFG ** get_next_nodes();

    /// @brief dataflow_list的getter
    /// @return 基本块内的数据流语句清单
    std::vector<Node_Dataflow *> & get_dataflow_list();

    // 定义友元函数，使其直接能访问private
    friend void LiveVariableAnalysis(ControlFlowGraph * _graph);

private:
    /// @brief 一个基本块最多有2个后继
    static const int MAX_NUM_OF_SON = 2;
    /// @brief 节点自己的邻接表
    Node_CFG * next_nodes[MAX_NUM_OF_SON] = {nullptr, nullptr};
    /**
     * @brief  直接后继节点的Label名称集合
     * @note 添加这个集合的原因：控制流图中有的Label对应的控制流块可能还没初始化，
     * 此时无法正确把Label转为控制流块，只能等到所有块初始化好后再进行Label转换。
     */
    std::set<std::string> son_labels;
    /// @brief 基本块内所有IR语句的列表（包含数据流信息）
    std::vector<Node_Dataflow *> dataflow_list;
};

/// @brief 控制流图
class ControlFlowGraph {
public:
    /// @brief 构造函数
    /// @param func 需要建立控制流图的函数
    ControlFlowGraph(Function * func);

    /// @brief 析构函数
    ~ControlFlowGraph();

    /// @brief 把一个Label贴到控制流块上
    /// @param label label名
    /// @param node 对应控制流块的指针
    /// @return label是否粘贴成功
    bool add_label_for_CFG(std::string label, Node_CFG * node);

    /// @brief 获得Label对应的控制流块
    /// @param label 要查找的Label
    /// @return 存在--返回控制流块 不存在--返回nullptr
    Node_CFG * get_CFG_from_label(std::string label);

    /// @brief 获取CFG的节点
    /// @return 节点表
    std::vector<Node_CFG *> & get_node_list();

    /// @brief Value表的getter
    /// @return Value表
    std::set<Value *> & get_value_list();

private:
    /// @brief Label到控制流节点的映射表
    std::map<std::string, Node_CFG *> LabelToNodeCFG;
    /// @brief 控制流图中的节点列表
    std::vector<Node_CFG *> node_list;
    /// @brief 控制流图中出现过的所有Value
    std::set<Value *> value_list;
};

/// @brief IR语句对应的数据流
struct Node_Dataflow {
public:
    /// @brief 构造函数
    /// @param _inst 原始IR语句
    Node_Dataflow(Instruction * _inst);
    /// @brief 析构函数
    ~Node_Dataflow();
    /// @brief 活跃变量分析
    std::set<Value *> liveIN;
    /// @brief 活跃变量分析
    std::set<Value *> liveOUT;
    /// @brief 对应的原始IR语句
    Instruction * inst;
    /// @brief def集
    std::set<Value *> def_set;
    /// @brief use集
    std::set<Value *> use_set;
};
