#include <stack>
#include <algorithm>
#include <bitset>
#include <queue>

#include "InterferenceGraph.h"
#include "PlatformArm64.h"
#include "AllocaInstruction.h"
#include "FuncCallInstruction.h"
#include "FloatType.h"
#include "MoveInstruction.h"
#include "Constant.h"

node_IG::node_IG(Value * _val)
{
    val = _val;
    vals.insert(_val);
}

node_IG::node_IG()
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
    // printf("删除节点\n");
    // assert(node->neighbors.size());
    for (node_IG * neighbor: node->neighbors) {
        neighbor->remove_neighbor(node);
    }
    // printf("从未染色列表里删除节点\n");
    uncolored_node_set.erase(node);
    // printf("删除完成\n");
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

    // printf("已生成控制流图\n");
    //  用完基本块表之后就可以删了节省空间
    func->clearBasicBlocks();
    printf("已删除不用的基本块表\n");

    // 加完新指令后也该重新调整IR编号
    func->renameIR();

    // printf("已释放临时基本块表\n");
    //  进行活跃变量分析，获得每条语句的DEF和USE集合
    LiveVariableAnalysis(graph_cfg);

    // printf("已经活跃变量分析\n");
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

void InterferenceGraph::add_mov_edge(node_IG * node1, node_IG * node2)
{
    node1->mov_neigh.insert(node2);
    node2->mov_neigh.insert(node1);
}

void InterferenceGraph::remove_mov_edge(node_IG * node1, node_IG * node2)
{
    node1->mov_neigh.erase(node2);
    node2->mov_neigh.erase(node1);
}

void InterferenceGraph::ExecuteCFG(ControlFlowGraph * graph)
{

    // 目前染色涉及的所有value
    std::set<Value *> all_value_in_cfg;

    // printf("所有指令列表:\n");
    for (Instruction * inst: graph->get_func()->getInterCode().getCode()) {
        printval(inst);
        /*
        printf("NUM OF OPERANDS:%d\n", inst->getOperandsNum());
        printf("DEF:\n");
        printset(inst->get_def_set());
        printf("USE:\n");
        printset(inst->get_use_set());
        printf("LIVEOUT:\n");
        printset(inst->get_liveout());
        printf("LIVEIN\n");
        printset(inst->get_livein());
        printf("\n");
        */
    }
    // printf("指令列表结束\n");
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
        /*printf("node:");
        printval(inst);
        printf("size of def_set:%d\n", int(inst->get_def_set().size()));
        printset(inst->get_def_set());
        printf("size of use_set:%d\n", int(inst->get_use_set().size()));
        printset(inst->get_use_set());
        printf("size of livein:%d\n", int(inst->get_livein().size()));
        printset(inst->get_livein());
        printf("size of liveout:%d\n", int(inst->get_liveout().size()));
        printset(inst->get_liveout());*/
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
        // printf("当前变量：");
        // printval(val);
        // printf("寄存器ID：%d\n", val->getRegId());
        newnode->color = RegIdToColor(val->getRegId());
        /*
        if (newnode->color == 28) {
            // printval(newnode->val);
            // std::cout << (is_float ? "true" : "false") << std::endl;
            assert(val == PlatformArm64::intRegVal[91]);
            assert(FloatType::getTypeFloat());
            assert(PlatformArm64::intRegVal[0]->getType());
            assert(PlatformArm64::intRegVal[91]->getType());
            std::cout << PlatformArm64::intRegVal[91]->getType()->getTypeID() << std::endl;
            // assert(PlatformArm64::intRegVal[91]->getType() == IntegerType::getTypeInt());
            assert(PlatformArm64::intRegVal[91]->getType() == FloatType::getTypeFloat());
            assert(val->getType() == FloatType::getTypeFloat());
        }
        */
        // assert(newnode->color != 28);
        // printf("newnode->color = val->getRegId(); %d\n", val->getRegId());
        if (newnode->color == -1) {
            uncolored_node_set.insert(newnode);
        }
    }
    // printf("所有干涉节点创建完成\n");

    // 统计移动边
    for (Instruction * inst: graph->get_func()->getInterCode().getCode()) {
        if (Instanceof(movinst, MoveInstruction *, inst)) {
            Value * result = movinst->getOperand(0);
            Value * src = movinst->getOperand(1);
            if (Instanceof(constsrc, Constant *, src)) {
                continue;
            }
            if (all_value_in_cfg.count(result) && all_value_in_cfg.count(src)) {
                mov_set.insert({value_to_ig[result], value_to_ig[src]});
                add_mov_edge(value_to_ig[result], value_to_ig[src]);
            }
        }
    }
    //  std::cout <<　uncolored_node_set.size() << std::endl;

    // 这是std::set版本的干涉图构建过程，时间复杂度是O(N * M * M * logN)，其中N为指令数目，M为活跃集合的size上限
    // 扫描函数里每条指令，获取每个时刻的活跃变量集合
    int i = 1;
    // printf("size of insts:%zu\n", graph->get_func()->getInterCode().getCode().size());
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
        // std::cout << "size of occupy:" << value_occupy.size() << std::endl;

        // printf("第%d次获取DEF、SET集合\n", i++);
        //  if (i == 2) {
        //  break;
        // }
        //   这些不同的量两两之间都是互斥的，不能在同一寄存器
        FOR_EACH_PAIR_IN_SET(value_occupy)
        {
            if (((*it1)->getRegId() != -1) && (*it2)->getRegId() != -1) {
                continue;
            }
            // printval(*it1);
            // printval(*it2);
            // assert(value_to_ig[*it1] && value_to_ig[*it2]);
            add_edge(value_to_ig[*it1], value_to_ig[*it2]);
        }
        /*
        for (auto it1 = (value_occupy).begin(); it1 != (value_occupy).end(); ++it1) {
            if ((*it1)->getRegId() != -1) {
                continue;
            }
            for (auto it2 = (value_occupy).begin(); it2 != (value_occupy).end(); ++it2) {
                // assert(it1 != it2);
                if (it1 == it2) {
                    continue;
                }
                // 因此在干涉图中连上一条边
                add_edge(value_to_ig[*it1], value_to_ig[*it2]);
            }
        }
        */
        // std::cout << "干涉边添加完毕" << std::endl;
    }

    // 下面的干涉图构建方法弃用，用回上面的。
    // 下面是BitSet版本的干涉图构建，时间复杂度O(N * N * (N/W + logN))，其中N是指令数，W = 32
    /*
    // 优化版本的干涉图产生过程
    // 每个Value都有一个位图，位图中每一位表示其是否在对应语句的活跃集合中出现
    std::map<Value *, std::bitset<10000>> live_set_of_value;

    // 集合所对应的位图坐标
    int index = 0;
    for (Instruction * inst: graph->get_func()->getInterCode().getCode()) {
        std::set<Value *> value_occupy = inst->get_liveout();
        merge_set(value_occupy, inst->get_def_set());
        // std::set<Value *> value_occupy = inst->get_livein();
        for (Value * val: value_occupy) {
            live_set_of_value[val].set(index);
        }
        index++;
    }
    printf("干涉图正在产生\n");
    for (Value * val1: all_value_in_cfg) {
        for (Value * val2: all_value_in_cfg) {
            if (val1 == val2) {
                continue;
            }
            if ((val1->getType() != val2->getType()) &&
                (val1->getType() == FloatType::getTypeFloat() || val2->getType() == FloatType::getTypeFloat())) {
                continue;
            }
            auto &bit1 = live_set_of_value[val1], &bit2 = live_set_of_value[val2];
            if ((bit1 & bit2).any()) {
                add_edge(value_to_ig[val1], value_to_ig[val2]);
            }
        }
    }
    */
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

        // 没有函数入口指令了
        /*
        // 函数入口指令
        if (inst->getOp() == IRInstOperator::IRINST_OP_ENTRY) {
            BasicBlock->addInst(inst);
        }
        */
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
    /*
    bool used[color_size];
    for (int i = 0; i < color_size; i++) {
        used[i] = false;
    }
    */
    std::vector<bool> used(color_size, false);
    for (node_IG * neighbor: node->neighbors) {
        if (neighbor->color != -1) {
            used.at(neighbor->color) = true;
        }
    }
    for (int color = 0; color < color_size; color++) {
        // printf("%d\n", color);
        if (!used[color]) {
            // printf("找到了\n");
            return color;
        }
    }
    // printf("没找到\n");
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

    // assert(remain_nodes.empty());
    //  std::cout << "最大度数" << remain_nodes[0]->degree() << std::endl;
    //   按照某序列依次给每个节点染上目前能染的最小编号颜色
    //   时间复杂度：O(m + n * min(c,n))，m为边数，c为颜色数，n为节点数
    for (node_IG * node: remain_nodes) {
        // printf("发生循环\n");

        // 尝试染上目前能染的最小编号颜色
        node->color = least_color_for_node(node, color_size);
        // printf("里程碑\n");
        //  中途有某个节点无颜色可用，则染色失败
        if (node->color == -1) {
            // printval(node->val);
            //  printf("%d\n", node->degree());
            return false;
        }
    }
    return true;
}

static bool backtrack_color(InterferenceGraph * graph, int color_size, std::set<node_IG *>::iterator iter)
{
    // printf("回溯法\n");

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

void InterferenceGraph::simplify(int color_size)
{
    // 先删除小度节点
    // 这里的queue只是为了代码方便，提前存储的一个uncolored_node_set的副本
    std::queue<node_IG *> uncolored_node_queue;
    for (node_IG * node: uncolored_node_set) {
        uncolored_node_queue.push(node);
    }
    while (uncolored_node_queue.empty() == false) {
        node_IG * node = uncolored_node_queue.front();
        uncolored_node_queue.pop();

        if (node->is_deleted || (node->mov_neigh.empty() == false)) {
            // 已删除的节点不做操作
            continue;
        }

        if (node->degree() < color_size) {
            // 删除操作
            node->is_deleted = true;
            this->remove_node(node);
            removed_nodes.push(node);
            // 再看邻居节点是否会因为度数减小而被删除
            for (node_IG * neighbor: node->neighbors) {
                uncolored_node_queue.push(neighbor);
            }
        }
    }
}

node_IG * InterferenceGraph::merge_node(node_IG * node1, node_IG * node2)
{
    node_IG * node_merged = new node_IG();
    node_merged->neighbors = node1->neighbors;
    merge_set(node_merged->neighbors, node2->neighbors);
    for (node_IG * neighbor: node_merged->neighbors) {
        neighbor->remove_neighbor(node1);
        neighbor->remove_neighbor(node2);
        neighbor->add_neighbor(node_merged);
    }
    return node_merged;
}

/*
bool InterferenceGraph::merge_node_briggs(node_IG * node1, node_IG * node2, int color_size)
{
    node_IG * node_merged = new node_IG();
    node_merged->neighbors = iter->first->neighbors;
    merge_set(node_merged->neighbors, iter->second->neighbors);
    if (node_merged->degree() < color_size) {
        merge_node_briggs();
        suc = true;
    }
}
*/

// 判定两个节点是否满足George条件（两个点有前后顺序差别）
bool george(node_IG * node1, node_IG * node2, int color_size)
{
    /*
    std::cout << "判定点" << std::endl;
    printval(node1->val);
    printval(node2->val);
    std::cout << std::endl;
    */

    for (node_IG * neighbor: node1->neighbors) {
        if (neighbor->degree() >= color_size) {
            return false;
        }
        if (neighbor->neighbors.count(node2) == 0) {
            return false;
        }
    }
    return true;
}

bool InterferenceGraph::merge_node_george(node_IG * node1, node_IG * node2, int color_size)
{
    if (george(node1, node2, color_size) || george(node2, node1, color_size)) {
        merge_node(node1, node2);
        node1->is_deleted = true;
        node2->is_deleted = true;
        std::cout << "接合了以下两个节点" << std::endl;
        std::cout << node1 << " " << node2 << std::endl;
        return true;
    }
    return false;
}

bool InterferenceGraph::coalesce(int color_size)
{
    bool suc = false;
    // assert(mov_set.size() == 1 || mov_set.size() == 0);
    for (auto iter = mov_set.begin(); iter != mov_set.end(); iter++) {
        // assert(iter->first);
        // assert(iter->second);
        if (iter->first->is_deleted || iter->second->is_deleted || iter->first->neighbors.count(iter->second)) {
            std::cout << "删除移动边" << std::endl;
            printval(iter->first->val);
            printval(iter->second->val);
            // assert(iter->first->is_deleted == false);
            // assert(iter->second->is_deleted == false);
            // assert(iter->first->neighbors.count(iter->second) == 0);
            std::cout << "完毕" << std::endl;
            remove_mov_edge(iter->first, iter->second);
            iter = mov_set.erase(iter);
            iter--;
        } else {
            suc |= merge_node_george(iter->first, iter->second, color_size);
            // suc = merge_node_briggs(iter->first, iter->second);
        }
    }
    return suc;
}

void InterferenceGraph::select(int color_size)
{
    // 然后，恢复小度节点并对这些小度节点着色
    // assert(removed_nodes.size() == 15);
    while (!removed_nodes.empty()) {
        node_IG * node = removed_nodes.top();
        removed_nodes.pop();
        this->restore_node(node);
        node->color = least_color_for_node(node, color_size);
    }
}

bool InterferenceGraph::color_graph(int color_size)
{
    // printf("开始染色\n");
    // 染色是否成功
    bool suc;

    switch (method_chosen) {
        case color_method::WELSH_POWELL:
            suc = welsh_powell(this, color_size);
            break;
        case color_method::BACKTRACK:
            suc = backtrack_color(this, color_size, this->uncolored_node_set.begin());
            break;
    }

    return suc;
}

int InterferenceGraph::ColorToRegId(int color, bool is_float)
{
    assert(color < 32);
    if (is_float) {
        return color + 63;
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
    if (regid >= 63) {
        return regid - 63;
    } else {
        if (regid >= PlatformArm64::CallerSaveRegNum) {
            return regid - 1;
        } else {
            return regid;
        }
    }
}