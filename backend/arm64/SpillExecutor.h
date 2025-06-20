#include "InterferenceGraph.h"

struct SpillExecutor {
    /// @brief 构造函数
    SpillExecutor();

    /// @brief 目前可用的溢出策略
    enum class spill_method { DEGREE_FIRST };

    /// @brief 干涉图度数优先的溢出方法
    /// @note 选择在干涉图里节点度数最高的一个变量溢出
    static void spill_degree_first(InterferenceGraph * _graph);

    /// @brief 将一个函数里某变量的出现全部替换为溢出
    /// @param func 函数
    /// @param val 变量
    static void spill(Function * func, Value * val);
};
