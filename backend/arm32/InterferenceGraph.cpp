#include <stack>
#include <algorithm>

#include "InterferenceGraph.h"

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
    node_set.erase(node);
}

void InterferenceGraph::restore_node(node_IG * node)
{
    node_set.insert(node);
    for (node_IG * neighbor: node->neighbors) {
        neighbor->add_neighbor(node);
    }
}

InterferenceGraph::InterferenceGraph(ControlFlowGraph * graph)
{

    // 从Value到干涉图节点的映射
    std::map<Value *, node_IG *> value_to_ig;

    for (Value * val: graph->get_value_list()) {
        node_IG * newnode = new node_IG(val);
        value_to_ig[val] = newnode;
        node_set.insert(newnode);
    }

    //扫描函数里每条指令，获取每个时刻的活跃变量集合
    for (Node_CFG * node_cfg: graph->get_node_list()) {
        for (Node_Dataflow * node_data: node_cfg->get_dataflow_list()) {

            // 某个指令位置下活跃着的量的集合（LiveOUT与def之并）
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

static bool welsh_powell(InterferenceGraph * graph, int color_size)
{
    std::vector<node_IG *> remain_nodes(graph->node_set.begin(), graph->node_set.end());

    // Welsh-Powell算法
    // 按照度数从大到小给剩余节点排序
    std::sort(remain_nodes.begin(), remain_nodes.end(), [&](node_IG * a, node_IG * b) {
        return a->degree() > b->degree();
    });

    // 按照某序列依次给每个节点染上目前能染的最小编号颜色
    // 时间复杂度：O(m + n * min(c,n))，m为边数，c为颜色数，n为节点数
    for (node_IG * node: remain_nodes) {
        std::vector<bool> used(color_size + 1, false);
        for (node_IG * neighbor: node->neighbors) {
            used[neighbor->color] = true;
        }
        for (int color = 1; color <= color_size; color++) {
            if (!used[color]) {
                node->color = color;
                break;
            }
        }

        // 中途有某个节点无颜色可用，则染色失败
        if (node->color == 0) {
            return false;
        }
    }
    return true;
}

static bool backtrack_color(InterferenceGraph * graph, int color_size, std::set<node_IG *>::iterator iter)
{

    // 目前回溯法之时间复杂度：O(m * c^n)，是指数级别，所以节点数只能为个位数，否则时间复杂度无法支持
    // 同时，不需要修改成非递归形式，因为递归深度很浅

    if (iter == graph->node_set.end()) {
        return true;
    }
    node_IG * node = *iter;
    std::vector<bool> used(color_size + 1, false);
    for (node_IG * neighbor: node->neighbors) {
        used[neighbor->color] = true;
    }
    for (int color = 1; color <= color_size; color++) {
        if (!used[color]) {
            node->color = color;
            if (backtrack_color(graph, color_size, ++iter)) {
                return true;
            }
            iter--;
        }
    }
    return false;
}

bool InterferenceGraph::color_graph(InterferenceGraph * graph, int color_size)
{
    // 暂时被移出干涉图的小度节点
    std::stack<node_IG *> removed_nodes;

    // 先删除小度节点
    for (node_IG * node: graph->node_set) {
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
            suc = backtrack_color(graph, color_size, graph->node_set.begin());
            break;
    }
}