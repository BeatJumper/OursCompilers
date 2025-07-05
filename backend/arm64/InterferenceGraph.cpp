#include <stack>
#include <algorithm>
#include <bitset>

#include "InterferenceGraph.h"
#include "PlatformArm64.h"
#include "AllocaInstruction.h"
#include "FuncCallInstruction.h"
#include "FloatType.h"

node_IG::node_IG(Value * _val) : val(_val)
{}

int node_IG::degree()
{
    return neighbors.size();
}

void node_IG::add_neighbor(node_IG * neighbor)
{
    neighbors.insert(neighbor);
}

void node_IG::remove_neighbor(node_IG * neighbor)
{
    neighbors.erase(neighbor);
}

void InterferenceGraph::add_edge(node_IG * node1, node_IG * node2)
{
    node1->add_neighbor(node2);
    node2->add_neighbor(node1);
}

void InterferenceGraph::remove_node(node_IG * node)
{
    for (node_IG * neighbor: node->neighbors) {
        neighbor->remove_neighbor(node);
    }
    uncolored_node_set.erase(node);
}

void InterferenceGraph::restore_node(node_IG * node)
{
    uncolored_node_set.insert(node);
    for (node_IG * neighbor: node->neighbors) {
        neighbor->add_neighbor(node);
    }
}

InterferenceGraph::InterferenceGraph(Function * func, bool is_float)
{
    this->is_float = is_float;

    // 调用基本块划分函数
    GenBasicBlocks(func);

    // 生成控制流图
    graph_cfg = new ControlFlowGraph(func);

    //  用完基本块表之后就可以删了节省空间
    func->clearBasicBlocks();

    // 加完新指令后也该重新调整IR编号
    func->renameIR();

    //  进行活跃变量分析，获得每条语句的DEF和USE集合
    LiveVariableAnalysis(graph_cfg);

    //  完成干涉图构建
    ExecuteCFG(graph_cfg);
}

InterferenceGraph::~InterferenceGraph()
{
    // 清理干涉图节点
    for (node_IG * node: node_set) {
        delete node;
    }
    node_set.clear();
    uncolored_node_set.clear();

    // 清理控制流图
    if (graph_cfg) {
        delete graph_cfg;
        graph_cfg = nullptr;
    }
}

void InterferenceGraph::ExecuteCFG(ControlFlowGraph * graph)
{
    // 从Value到干涉图节点的映射
    std::map<Value *, node_IG *> value_to_ig;
    std::set<Value *> all_value_in_cfg;

    for (Instruction * inst: graph->get_func()->getInterCode().getCode()) {
        for (Value * val: inst->get_def_set()) {
            if ((val->getType() == FloatType::getTypeFloat()) == is_float) {
                all_value_in_cfg.insert(val);
            }
        }
        for (Value * val: inst->get_use_set()) {
            if ((val->getType() == FloatType::getTypeFloat()) == is_float) {
                all_value_in_cfg.insert(val);
            }
        }
    }

    // 为每个Value都创建一个干涉图节点
    for (Value * val: all_value_in_cfg) {
        // 跳过alloca指令，它们不应该参与寄存器分配
        if (dynamic_cast<AllocaInstruction *>(val)) {
            continue;
        }

        node_IG * newnode = new node_IG(val);
        value_to_ig[val] = newnode;
        node_set.insert(newnode);
        // 已经提前指定了寄存器的Value对应的干涉图节点应该预先染色
        newnode->color = RegIdToColor(val->getRegId());
        
        if (newnode->color == -1) {
            uncolored_node_set.insert(newnode);
        }
    }

    // 这是std::set版本的干涉图构建过程，时间复杂度是O(N * M * M * logN)，其中N为指令数目，M为活跃集合的size上限
    // 扫描函数里每条指令，获取每个时刻的活跃变量集合
    for (Instruction * inst: graph->get_func()->getInterCode().getCode()) {
        std::set<Value *> value_occupy = inst->get_liveout();
        merge_set(value_occupy, inst->get_def_set());
        //  手动循环的安全版本
        for (auto it = value_occupy.begin(); it != value_occupy.end();) {
            if (all_value_in_cfg.count(*it) == 0) {
                it = value_occupy.erase(it); // erase 返回下一个有效迭代器
            } else {
                ++it;
            }
        }
        //   这些不同的量两两之间都是互斥的，不能在同一寄存器
        FOR_EACH_PAIR_IN_SET(value_occupy)
        {
            if (((*it1)->getRegId() != -1) && (*it2)->getRegId() != -1) {
                continue;
            }
            add_edge(value_to_ig[*it1], value_to_ig[*it2]);
        }
    }
}

void InterferenceGraph::flush_all_color()
{
    for (node_IG * node: node_set) {
        node->color = -1;
    }
}

/// @brief 为函数找出各基本块所包含的指令，并打包为基本块存入链表
/// @param func 函数指针
void InterferenceGraph::GenBasicBlocks(Function * func)
{
    InterCode * BasicBlock = new InterCode();
    // 遍历func所有指令
    for (auto inst: func->getInterCode().getInsts()) {
        // 找出所有首指令
        BasicBlock->addInst(inst);
        // 遇到跳转指令就分块
        if (inst->getOp() == IRInstOperator::IRINST_OP_GOTO || inst->getOp() == IRInstOperator::IRINST_OP_BRANCH) {
            func->addBasicBlock(BasicBlock);
            BasicBlock = new InterCode();
        }
    }
    // 添加最后一个基本块
    if (!BasicBlock->getInsts().empty()) {
        func->addBasicBlock(BasicBlock);
    }
}

static int least_color_for_node(node_IG * node, int color_size)
{
    std::vector<bool> used(color_size, false);
    for (node_IG * neighbor: node->neighbors) {
        if (neighbor->color != -1) {
            used.at(neighbor->color) = true;
        }
    }
    for (int color = 0; color < color_size; color++) {
        if (!used[color]) {
            return color;
        }
    }
    return -1;
}

static bool welsh_powell(InterferenceGraph * graph, int color_size)
{
    std::vector<node_IG *> remain_nodes(graph->uncolored_node_set.begin(), graph->uncolored_node_set.end());
    // Welsh-Powell算法
    // 按照度数从大到小给剩余节点排序
    std::sort(remain_nodes.begin(), remain_nodes.end(), [&](node_IG * a, node_IG * b) {
        return a->degree() > b->degree();
    });

    //   按照某序列依次给每个节点染上目前能染的最小编号颜色
    //   时间复杂度：O(m + n * min(c,n))，m为边数，c为颜色数，n为节点数
    for (node_IG * node: remain_nodes) {

        // 尝试染上目前能染的最小编号颜色
        node->color = least_color_for_node(node, color_size);
        //  中途有某个节点无颜色可用，则染色失败
        if (node->color == -1) {
            return false;
        }
    }
    return true;
}

static bool backtrack_color(InterferenceGraph * graph, int color_size, std::set<node_IG *>::iterator iter)
{

    // 目前回溯法之时间复杂度：O(m * c^n)，是指数级别，所以节点数只能为个位数，否则时间复杂度无法支持
    // 同时，不需要修改成非递归形式，因为递归深度很浅

    if (iter == graph->uncolored_node_set.end()) {
        return true;
    }
    node_IG * node = *iter;
    std::vector<bool> used(color_size, false);
    for (node_IG * neighbor: node->neighbors) {
        used[neighbor->color] = true;
    }
    for (int color = 0; color < color_size; color++) {
        if (!used[color]) {
            node->color = color;
            if (backtrack_color(graph, color_size, ++iter)) {
                return true;
            }
            iter--;
        }
    }
    node->color = -1;
    return false;
}

bool InterferenceGraph::color_graph(InterferenceGraph * graph, int color_size)
{
    //  暂时被移出干涉图的小度节点
    std::stack<node_IG *> removed_nodes;

    // 先删除小度节点

    // 这里的queue只是为了代码方便，提前存储的一个uncolored_node_set的副本
    std::vector<node_IG *> uncolored_node_queue(graph->uncolored_node_set.begin(), graph->uncolored_node_set.end());
    for (node_IG * node: uncolored_node_queue) {
        if (node->degree() < color_size) {
            graph->remove_node(node);
            removed_nodes.push(node);
        }
    }
    // 染色是否成功
    bool suc;

    switch (method_chosen) {
        case color_method::WELSH_POWELL:
            suc = welsh_powell(graph, color_size);
            break;
        case color_method::BACKTRACK:
            suc = backtrack_color(graph, color_size, graph->uncolored_node_set.begin());
            break;
    }

    // 然后，恢复小度节点并对这些小度节点着色
    while (!removed_nodes.empty()) {
        node_IG * node = removed_nodes.top();
        removed_nodes.pop();
        graph->restore_node(node);
        if (suc) {
            node->color = least_color_for_node(node, color_size);
        }
    }
    return suc;
}

int InterferenceGraph::ColorToRegId(int color, bool is_float)
{
    assert(color < 32);
    if (is_float) {
        return color + 64;
    } else {
        if (color < PlatformArm64::CallerSaveRegNum) {
            return color;
        } else {
            return color + 1;
        }
    }
}

int InterferenceGraph::RegIdToColor(int regid)
{
    if (regid == -1) {
        return -1;
    }
    if (regid >= 64) {
        return regid - 64;
    } else {
        if (regid >= PlatformArm64::CallerSaveRegNum) {
            return regid - 1;
        } else {
            return regid;
        }
    }
}