///
/// @file MemVariable.h
/// @brief 内存变量，用于栈内变量的描述。用于后端处理
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#pragma once

#include "Value.h"
#include "IRConstant.h"

#include <cassert>

/// @brief 内存值，必须在内存中
class MemVariable : public Value {

    friend class Function;

public:
    /// @brief 创建内存Value
    /// \param val
    explicit MemVariable(Type * _type) : Value(_type)
    {}

    ///
    /// @brief @brief 如是内存变量型Value，则获取基址寄存器和偏移
    /// @param regId 寄存器编号
    /// @param offset 相对偏移
    /// @return true 是内存型变量
    /// @return false 不是内存型变量
    ///
    bool getMemoryAddr(int32_t * _regId = nullptr, int64_t * _offset = nullptr) override
    {
        // 肯定是内存地址

        if (_regId) {
            *_regId = this->baseRegNo;
        }
        if (_offset) {
            *_offset = this->offset;
        }

        return true;
    }

    ///
    /// @brief 设置内存寻址的基址寄存器和偏移
    /// @param _regId 基址寄存器编号
    /// @param _offset 偏移
    ///
    void setMemoryAddr(int32_t _regId, int64_t _offset)
    {
        printf("调用setMemoryAddr中，对象地址: %p\n", this);

        // 检查this指针是否有效
        assert(this != NULL && "setMemoryAddr: this指针为空!");

        // 检查参数是否合法
        printf("setMemoryAddr: 寄存器号=%d, 偏移量=%ld\n", _regId, _offset);

        // 保存原始值用于调试
        int32_t oldReg = this->baseRegNo;
        int64_t oldOffset = this->offset;

        try {
            // 尝试更新基址寄存器
            printf("setMemoryAddr: 准备更新baseRegNo\n");
            baseRegNo = _regId;
            printf("baseRegNo赋值成功，从%d更新为%d\n", oldReg, baseRegNo);

            // 更新基址寄存器名称
            char regName[16];
            snprintf(regName, sizeof(regName), "r%d", _regId);
            baseRegName = regName;
            printf("baseRegName更新为: %s\n", baseRegName.c_str());

            // 尝试更新偏移量
            printf("setMemoryAddr: 准备更新offset\n");
            offset = _offset;
            printf("offset赋值成功，从%ld更新为%ld\n", oldOffset, offset);

            printf("setMemoryAddr执行完毕\n");
        } catch (const std::exception & e) {
            printf("setMemoryAddr抛出异常: %s\n", e.what());
            // 恢复原始值
            baseRegNo = oldReg;
            offset = oldOffset;
        }
    }

private:
    /// @brief 变量在栈内的偏移量，对于全局变量默认为0，临时变量没有意义
    int64_t offset = 0;

    /// @brief 栈内寻找时基址寄存器编号
    int32_t baseRegNo = -1;

    /// @brief 栈内寻找时基址寄存器名字
    std::string baseRegName;

    ///
    /// @brief 变量加载到寄存器中时对应的寄存器编号
    ///
    int32_t loadRegNo = -1;
};
