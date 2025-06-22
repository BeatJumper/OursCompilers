/// @file CodeGeneratorArm32.cpp
/// @brief ARM64的后端处理实现

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "Function.h"
#include "IRCode.h"
#include "Module.h"
#include "PlatformArm64.h"
#include "CodeGeneratorArm64.h"
#include "InstSelectorArm64.h"
#include "SimpleRegisterAllocator.h"
#include "ILocArm64.h"
#include "RegVariable.h"
#include "FuncCallInstruction.h"
#include "ArgInstruction.h"
#include "MoveInstruction.h"
#include "Instruction.h"
#include "InterferenceGraph.h"

/// @brief 构造函数
/// @param tab 符号表
CodeGeneratorArm64::CodeGeneratorArm64(Module * _module) : CodeGeneratorAsm(_module)
{}

/// @brief 析构函数
CodeGeneratorArm64::~CodeGeneratorArm64()
{}

/// @brief 产生汇编头部分
void CodeGeneratorArm64::genHeader()
{
    //指定目标架构为 ARMv8-A
    fprintf(fp, "%s\n", ".arch armv8-a");
    //代码段
    fprintf(fp, "%s\n", ".text");
    //代码段按四字节对齐
    fprintf(fp, "%s\n", ".align 2");
    // 若有浮点运算需求，可添加如下指令支持高级SIMD和浮点单元
    // fpintf(fp, "%s\n", ".fpu neon-fp-armv8");
    //生成的汇编代码将使用 ARM 指令集的指令
    fprintf(fp, "%s\n", ".cpu generic+fp+simd");

    fprintf(fp, "\n");
}

/// @brief 全局变量Section，主要包含初始化的和未初始化过的
void CodeGeneratorArm64::genDataSection()
{
    // 生成代码段
    fprintf(fp, ".text\n");

    bool bssStarted = false;
    bool dataStarted = false;

    // 目前不支持全局变量和静态变量，以及字符串常量
    // 全局变量分两种情况：初始化的全局变量和未初始化的全局变量
    for (auto var: module->getGlobalVariables()) {

        if (var->isInBSSSection()) {

            // 在BSS段的全局变量，可以包含初值全是0的变量
            fprintf(fp, ".comm %s, %d, %d\n", var->getName().c_str(), var->getType()->getSize(), var->getAlignment());
        } else {
            // 有初值的全局变量
            if (!dataStarted) {
                fprintf(fp, ".data\n");
                dataStarted = true;
            }

            fprintf(fp, ".global %s\n", var->getName().c_str());
            fprintf(fp, ".type %s, %%object\n", var->getName().c_str());
            fprintf(fp, "%s\n", var->getName().c_str());
            // TODO 后面设置初始化的值，具体请参考ARM的汇编
        }
    }
}

///
/// @brief 获取IR变量相关信息字符串
/// @param str
///
void CodeGeneratorArm64::getIRValueStr(Value * val, std::string & str)
{
    std::string name = val->getName();
    std::string IRName = val->getIRName();
    int32_t regId = val->getRegId();
    int32_t baseRegId;
    int64_t offset;
    std::string showName;

    if (name.empty() && (!IRName.empty())) {
        showName = IRName;
    } else if ((!name.empty()) && IRName.empty()) {
        showName = IRName;
    } else if ((!name.empty()) && (!IRName.empty())) {
        showName = name + ":" + IRName;
    } else {
        showName = "";
    }

    if (regId != -1) {
        // 寄存器
        str += "\t@ " + showName + ":" + PlatformArm64::regName[regId];
    } else if (val->getMemoryAddr(&baseRegId, &offset)) {
        // 栈内寻址，[fp,#4]
        str += "\t@ " + showName + ":[" + PlatformArm64::regName[baseRegId] + ",#" + std::to_string(offset) + "]";
    }
}

/// @brief 针对函数进行汇编指令生成，放到.text代码段中
/// @param func 要处理的函数
void CodeGeneratorArm64::genCodeSection(Function * func)
{

    // 寄存器分配以及栈内局部变量的站内地址重新分配
    registerAllocation(func);
    printf("寄存器分配完成\n");

    // 获取函数的指令列表
    std::vector<Instruction *> & IrInsts = func->getInterCode().getInsts();
    printf("成功获取指令列表，指令数量：%d\n", int(IrInsts.size()));

    // 汇编指令输出前要确保Label的名字有效，必须是程序级别的唯一，而不是函数内的唯一。要全局编号。
    for (auto inst: IrInsts) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_LABEL) {
            inst->setName(IR_LABEL_PREFIX + std::to_string(labelIndex++));
        }
    }
    // ILOC代码序列
    ILocArm64 iloc(module);

    // 指令选择生成汇编指令
    InstSelectorArm64 instSelector(IrInsts, iloc, func, simpleRegisterAllocator);
    instSelector.setShowLinearIR(this->showLinearIR);
    iloc.allocStack(func, ARM64_TMP_REG_NO);
    instSelector.run();
    printf("汇编已生成\n");

    // 删除无用的Label指令
    iloc.deleteUsedLabel();
    printf("删除无用的Label指令\n");

    // ILOC代码输出为汇编代码
    // 函数入口标签 - 直接生成全局标签
    fprintf(fp, ".global %s\n", func->getName().c_str());
    fprintf(fp, ".type %s, %%function\n", func->getName().c_str());
    fprintf(fp, ".align %d\n", func->getAlignment());
    fprintf(fp, "%s:\n", func->getName().c_str()); // 直接输出函数名标签
    printf("函数入口标签\n");

    // 开启时输出IR指令作为注释
    if (this->showLinearIR) {
        printf("进入if\n");
        // 输出有关局部变量的注释，便于查找问题
        for (auto localVar: func->getVarValues()) {
            printf("循环中\n");
            std::string str;
            getIRValueStr(localVar, str);
            if (!str.empty()) {
                fprintf(fp, "%s\n", str.c_str());
            }
        }
        printf("输出有关局部变量的注释\n");

        // 输出指令关联的临时变量信息
        for (auto inst: func->getInterCode().getInsts()) {
            if (inst->hasResultValue()) {
                std::string str;
                getIRValueStr(inst, str);
                if (!str.empty()) {
                    fprintf(fp, "%s\n", str.c_str());
                }
            }
        }
        printf("输出指令关联的临时变量信息\n");
    }
    printf("output\n");
    iloc.outPut(fp);
}

/// @brief 寄存器分配
/// @param func 函数指针
void CodeGeneratorArm64::registerAllocation(Function * func)
{
    // 内置函数不需要处理
    if (func->isBuiltin()) {
        return;
    }

    // 最简单/朴素的寄存器分配策略：局部变量和临时变量都保存在栈内，全局变量在静态存储.data区中
    // R0,R1,R2和R3寄存器不需要保护，可直接使用
    // SP寄存器预留，不需要保护，但需要保证值的正确性
    // R4-R10, fp(11), lx(14)都需要保护，没有函数调用的函数可不用保护lx寄存器
    // 被保留的寄存器主要有：
    //  (1) FP寄存器用于栈寻址，即R11
    //  (2) LX寄存器用于函数调用，即R14。没有函数调用的函数可不用保护lx寄存器
    //  (3) R10寄存器用于立即数过大时要通过寄存器寻址，这里简化处理进行预留

    // 创建干涉图
    InterferenceGraph * graph_ig = new InterferenceGraph(func);

    // TODO 考虑溢出
    // 进行染色
    InterferenceGraph::color_graph(graph_ig, PlatformArm64::maxUsableRegNum);

    std::vector<int32_t> & protectedRegNo = func->getProtectedReg();

    protectedRegNo.push_back(ARM64_FP_REG_NO);
    if (func->getExistFuncCall()) {
        protectedRegNo.push_back(ARM64_LX_REG_NO);
    }
    printf("寄存器分配中段\n");

    // 调整函数调用指令，主要是前8个寄存器传值，后面用栈传递
    // 为了更好的进行寄存器分配，可以进行对函数调用的指令进行预处理
    // 当然也可以不做处理，不过性能更差。这个处理是可选的。
    adjustFuncCallInsts(func);
    printf("调整函数调用指令\n");
    // 为局部变量和临时变量在栈内分配空间，指定偏移，进行栈空间的分配
    stackAlloc(func);
    printf("为局部变量和临时变量在栈内分配空间\n");
    // 函数形参要求前8个寄存器分配，后面的参数采用栈传递，实现实参的值传递给形参
    // 这一步是必须的
    adjustFormalParamInsts(func);
    printf("函数形参\n");
    // GenBasicBlocks(func);
    // printf("基本块划分成功\n");

#if 0
    // 临时输出调整后的IR指令，用于查看当前的寄存器分配、栈内变量分配、实参入栈等信息的正确性
    std::string irCodeStr;
    func->toString(irCodeStr);
    std::cout << irCodeStr << std::endl;
#endif
}

/// @brief 寄存器分配前对函数内的指令进行调整，以便方便寄存器分配
/// @param func 要处理的函数
void CodeGeneratorArm64::adjustFormalParamInsts(Function * func)
{
    // 函数形参的前8个实参值临时变量采用的是寄存器传值
    // 前8个之后通过栈传递

    // 请注意这里所得的所有形参都是对应的实参的值关联的临时变量
    // 如果不是不能使用这里的代码
    auto & params = func->getParams();

    // 形参的前8个通过寄存器来传值X0-X7
    for (int k = 0; k < (int) params.size() && k <= 7; k++) {

        // 前四个设置分配寄存器

        params[k]->setRegId(k);
    }

    // 根据ARM版C语言的调用约定，除前8个外的实参进行值传递，逆序入栈
    int64_t fp_esp = func->getMaxDep() + (func->getProtectedReg().size() * 8);
    for (int k = 8; k < (int) params.size(); k++) {

        // 目前假定变量大小都是4字节。实际要根据类型来计算

        params[k]->setMemoryAddr(ARM64_FP_REG_NO, fp_esp);

        // 增加8字节
        fp_esp += 8;
    }
}

/// @brief 寄存器分配前对函数内的指令进行调整，以便方便寄存器分配
/// @param func 要处理的函数
void CodeGeneratorArm64::adjustFuncCallInsts(Function * func)
{
    std::vector<Instruction *> newInsts;

    // 当前函数的指令列表
    auto & insts = func->getInterCode().getInsts();
    int len = insts.size();
    printf("获取指令列表成功:%d个\n", len);
    int i = 0;
    // 函数返回值用X0寄存器，若函数调用有返回值，则赋值X0到对应寄存器
    for (auto pIter = insts.begin(); pIter != insts.end(); pIter++) {
        i++;
        printf("循环第%d层\n", i);
        // 检查是否是函数调用指令，并且含有返回值
        if (Instanceof(callInst, FuncCallInstruction *, *pIter)) {
            printf("处理函数调用指令中\n");
            // 实参前8个要寄存器传值，其它参数通过栈传递

            // 前8个的后面参数采用栈传递
            int esp = 0;
            for (int32_t k = 8; k < callInst->getOperandsNum(); k++) {

                auto arg = callInst->getOperand(k);

                // 新建一个内存变量，用于栈传值到形参变量中
                LocalVariable * newVal = func->newLocalVarValue(IntegerType::getTypeInt());
                newVal->setMemoryAddr(ARM64_SP_REG_NO, esp);
                esp += 8;

                Instruction * assignInst = new MoveInstruction(func, newVal, arg);

                callInst->setOperand(k, newVal);

                // 函数调用指令前插入后，pIter仍指向函数调用指令
                pIter = insts.insert(pIter, assignInst);
                pIter++;
            }
            printf("栈传递参数处理结束\n");

            for (int k = 0; k < callInst->getOperandsNum() && k < 8; k++) {

                // 检查实参的类型是否是临时变量。
                // 如果是临时变量，该变量可更改为寄存器变量即可，或者设置寄存器号
                // 如果不是，则必须开辟一个寄存器变量，然后赋值即可
                auto arg = callInst->getOperand(k);

                if (arg->getRegId() == k) {
                    // 则说明寄存器已经是实参传递的寄存器，不用创建赋值指令
                    continue;
                } else {
                    // 创建临时变量，指定寄存器

                    Instruction * assignInst =
                        new MoveInstruction(func, PlatformArm64::intRegVal[k], callInst->getOperand(k));

                    callInst->setOperand(k, PlatformArm64::intRegVal[k]);

                    // 函数调用指令前插入后，pIter仍指向函数调用指令
                    pIter = insts.insert(pIter, assignInst);
                    pIter++;
                }
            }
            printf("实参处理结束\n");
            int callOperandsNum = callInst->getOperandsNum();
            printf("函数参数个数：%d\n", callOperandsNum);
            for (int k = 0; k < callOperandsNum; k++) {
                printf("第%d层循环\n", k);
                auto arg = callInst->getOperand(k);

                // 再产生ARG指令
                pIter = insts.insert(pIter, new ArgInstruction(func, arg));
                pIter++;
            }
            printf("参数传递指令处理结束\n");

            // 有arg指令后可不用参数，展示不删除
            // args.clear();

            // 赋值指令
            if (callInst->hasResultValue()) {

                if (callInst->getRegId() == 0) {
                    // 结果变量的寄存器和返回值寄存器一样，则什么都不需要做
                    ;
                } else {
                    // 其它情况，需要产生赋值指令
                    // 新建一个赋值操作
                    Instruction * assignInst = new MoveInstruction(func, callInst, PlatformArm64::intRegVal[0]);

                    // 函数调用指令的下一个指令的前面插入指令，因为有Exit指令，+1肯定有效
                    pIter = insts.insert(pIter + 1, assignInst);
                }
            }
            printf("返回值处理结束\n");
        }
    }
}

/// @brief 栈空间分配
/// @param func 要处理的函数
void CodeGeneratorArm64::stackAlloc(Function * func)
{
    // 遍历函数内的所有指令，查找没有寄存器分配的变量，然后进行栈内空间分配

    // 这里对临时变量和局部变量都在栈上进行分配,但形参对应实参的临时变量(FormalParam类型)不需要考虑

    int64_t sp_esp = 0;

    // 只处理未分配到寄存器的局部变量
    for (auto local: func->getVarValues()) {
        if (local->getRegId() != -1) {
            continue; // 跳过已分配寄存器的变量
        }
        // 对齐到8字节边界
        sp_esp = (sp_esp + 7) & ~7;
        local->setMemoryAddr(ARM64_FP_REG_NO, -sp_esp);
        sp_esp += local->getType()->getSize();
    }

    printf("开始处理Alloca\n");
    // 遍历指令中的alloca结果
    for (auto inst: func->getInterCode().getInsts()) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_ALLOCA) {
            // 确保alloca的结果变量被分配空间
            Value * result = inst->getOperand(0);
            LocalVariable * localResult = dynamic_cast<LocalVariable *>(result);

            if (!localResult->getMemoryAddr()) {
                // 获取分配大小（默认为指针大小）
                int64_t size = localResult->getType()->getSize();
                if (size == 0) {
                    size = 8; // 默认指针大小
                }

                // 8字节对齐
                size = (size + 7) & ~7;

                // 分配栈空间
                sp_esp -= size; // 栈向下增长
                localResult->setMemoryAddr(ARM64_FP_REG_NO, sp_esp);
            }
        }

        // ... 其他类型指令处理 ...
    }

    // 遍历指令中临时变量
    for (auto inst: func->getInterCode().getInsts()) {

        if (inst->hasResultValue()) {
            // 有值
            int32_t size = inst->getType()->getSize();

            // 64位ARM平台按照8字节的大小整数倍分配局部变量
            size += (8 - size % 8) % 8;

            // 临时变量偏移设置
            inst->setMemoryAddr(ARM64_FP_REG_NO, sp_esp);

            // 累计当前作用域大小
            sp_esp += size;
        }
    }

    // 设置函数的最大栈帧深度，在加上实参内存传值的空间
    // 请注意若支持浮点数，则必须保持栈内空间8字节对齐
    func->setMaxDep(sp_esp);
}
