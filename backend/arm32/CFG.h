#include <vector>
#include <set>
#include <map>

#include "Function.h"

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

private:
    /// @brief 节点对应的基本块编号
    int no;
    /// @brief 节点自己的邻接表
    std::set<Node_CFG *> next_nodes;
    /// @brief def集
    std::set<Value *> def_set;
    /// @brief use集
    std::set<Value *> use_set;
    /**
     * @brief  直接后继节点的Label名称集合
     * @note 添加这个集合的原因：控制流图中有的Label对应的控制流块可能还没初始化，
     * 此时无法正确把Label转为控制流块，只能等到所有块初始化好后再进行Label转换。
     */
    std::set<std::string> son_labels;
};

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

private:
    /// @brief Label到控制流节点的映射表
    std::map<std::string, Node_CFG *> LabelToNodeCFG;
    /// @brief 控制流图中的节点列表
    std::vector<Node_CFG *> node_list;
};