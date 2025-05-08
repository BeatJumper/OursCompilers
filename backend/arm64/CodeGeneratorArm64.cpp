///
/// @file CodeGeneratorArm32.cpp
/// @brief ARM32的后端处理实现
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// </table>
///
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
    fprintf(fp, "%s\n", ".fpu neonfp-armv8");
	//生成的汇编代码将使用 ARM 指令集的指令
    fprintf(fp, "%s\n", ".arm64");
}

/// @brief 全局变量Section，主要包含初始化的和未初始化过的
void CodeGeneratorArm64::genDataSection()
{
    // 生成代码段
    fprintf(fp, ".text\n");

    // 可直接操作文件指针fp进行写操作

    // 目前不支持全局变量和静态变量，以及字符串常量
    // 全局变量分两种情况：初始化的全局变量和未初始化的全局变量
    // TODO 这里先处理未初始化的全局变量
    for (auto var: module->getGlobalVariables()) {

        if (var->isInBSSSection()) {

            // 在BSS段的全局变量，可以包含初值全是0的变量
            fprintf(fp, ".comm %s, %d, %d\n", var->getName().c_str(), var->getType()->getSize(), var->getAlignment());
        } else {

            // 有初值的全局变量
            fprintf(fp, ".global %s\n", var->getName().c_str());
            fprintf(fp, ".data\n");
            fprintf(fp, ".align %d\n", var->getAlignment());
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
    printf("成功获取指令列表，指令数量：%d\n",int(IrInsts.size()));

    // 汇编指令输出前要确保Label的名字有效，必须是程序级别的唯一，而不是函数内的唯一。要全局编号。
    for (auto inst: IrInsts) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_LABEL) {
            printf("if语句开始执行\n");
            inst->setName(IR_LABEL_PREFIX + std::to_string(labelIndex++));
            printf("if语句执行完成\n");
        }
    }
	printf("for循环执行完成\n");
    // ILOC代码序列
    ILocArm64 iloc(module);
    printf("ILOC代码序列\n");

    // 指令选择生成汇编指令
    InstSelectorArm64 instSelector(IrInsts, iloc, func, simpleRegisterAllocator);
    printf("instSelector\n");
    instSelector.setShowLinearIR(this->showLinearIR);
    printf("setShowLinearIR\n");
    instSelector.run();
    printf("汇编已生成\n");

    // 删除无用的Label指令
    iloc.deleteUsedLabel();

    // ILOC代码输出为汇编代码
    fprintf(fp, ".align %d\n", func->getAlignment());
    fprintf(fp, ".global %s\n", func->getName().c_str());
    fprintf(fp, ".type %s, %%function\n", func->getName().c_str());
    fprintf(fp, "%s:\n", func->getName().c_str());

    // 开启时输出IR指令作为注释
    if (this->showLinearIR) {

        // 输出有关局部变量的注释，便于查找问题
        for (auto localVar: func->getVarValues()) {
            std::string str;
            getIRValueStr(localVar, str);
            if (!str.empty()) {
                fprintf(fp, "%s\n", str.c_str());
            }
        }

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
    }

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

    std::vector<int32_t> & protectedRegNo = func->getProtectedReg();

    protectedRegNo.push_back(ARM64_TMP_REG_NO);
    protectedRegNo.push_back(ARM64_FP_REG_NO);
    if (func->getExistFuncCall()) {
        protectedRegNo.push_back(ARM64_LX_REG_NO);
    }

    // 调整函数调用指令，主要是前8个寄存器传值，后面用栈传递
    // 为了更好的进行寄存器分配，可以进行对函数调用的指令进行预处理
    // 当然也可以不做处理，不过性能更差。这个处理是可选的。
    adjustFuncCallInsts(func);
    // 为局部变量和临时变量在栈内分配空间，指定偏移，进行栈空间的分配
    stackAlloc(func);

    // 函数形参要求前8个寄存器分配，后面的参数采用栈传递，实现实参的值传递给形参
    // 这一步是必须的
    adjustFormalParamInsts(func);

    GenBasicBlocks(func);
    printf("基本块划分成功\n");

#if 0
    // 临时输出调整后的IR指令，用于查看当前的寄存器分配、栈内变量分配、实参入栈等信息的正确性
    std::string irCodeStr;
    func->toString(irCodeStr);
    std::cout << irCodeStr << std::endl;
#endif
}

/// @brief 划分基本块
/// @param func 函数指针
void CodeGeneratorArm64::GenBasicBlocks(Function * func)
{
    // 首先获取函数的所有指令
    std::vector<Instruction *> insts = func->getInterCode().getInsts();

    InterCode * BasicBlock = new InterCode();
    Instruction * lastInst = nullptr;
    // 遍历func所有指令
    for (auto inst: insts) {
        // 找出所有首指令
        // 函数入口指令
        if (inst->getOp() == IRInstOperator::IRINST_OP_ENTRY) {
            BasicBlock->addInst(inst);
        }
        // 无条件分支指令
        else if (inst->getOp() == IRInstOperator::IRINST_OP_GOTO) {
            BasicBlock->addInst(inst);
            func->addBasicBlock(BasicBlock);
            BasicBlock->deleteInst();
        }
        // 紧跟在一个条件或无条件转移指令之后的指令
        else if (lastInst != nullptr && lastInst->getOp() == IRInstOperator::IRINST_OP_GOTO) {
            // 如果当前基本块不为空，添加到函数中
            if (!BasicBlock->getInsts().empty()) {
                func->addBasicBlock(BasicBlock);
            }
            BasicBlock->deleteInst();
            BasicBlock->addInst(inst);
        }
        else {
            BasicBlock->addInst(inst);
        }
        // 记录前一条指令
        lastInst = inst;
    }
    // 添加最后一个基本块
	if (!BasicBlock->getInsts().empty()) {
        func->addBasicBlock(BasicBlock);
    }

    // 释放暂存基本块的内存
    BasicBlock->deleteInst();
    free(BasicBlock);
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

    // 函数返回值用X0寄存器，若函数调用有返回值，则赋值X0到对应寄存器
    for (auto pIter = insts.begin(); pIter != insts.end(); pIter++) {

        // 检查是否是函数调用指令，并且含有返回值
        if (Instanceof(callInst, FuncCallInstruction *, *pIter)) {

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

            for (int k = 0; k < callInst->getOperandsNum(); k++) {

                auto arg = callInst->getOperand(k);

                // 再产生ARG指令
                pIter = insts.insert(pIter, new ArgInstruction(func, arg));
                pIter++;
            }

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

    // 获取函数变量列表
    std::vector<LocalVariable *> & vars = func->getVarValues();

    for (auto var: vars) {

        // 对于简单类型的寄存器分配策略，假定临时变量和局部变量都保存在栈中，属于内存
        // 而对于图着色等，临时变量一般是寄存器，局部变量也可能修改为寄存器
        // TODO 考虑如何进行分配使得临时变量尽量保存在寄存器中，作为优化点考虑

        // regId不为-1，则说明该变量分配为寄存器
        // baseRegNo不等于-1，则说明该变量肯定在栈上，属于内存变量，之前肯定已经分配过
        if ((var->getRegId() == -1) && (!var->getMemoryAddr())) {

            // 该变量没有分配寄存器

            int64_t size = var->getType()->getSize();

            // 64位ARM平台按照8字节的大小整数倍分配局部变量
            size += (8 - size % 8) % 8;

            // 这里要注意检查变量栈的偏移范围。一般采用机制寄存器+立即数方式间接寻址
            // 若立即数满足要求，可采用基址寄存器+立即数变量的方式访问变量
            // 否则，需要先把偏移量放到寄存器中，然后机制寄存器+偏移寄存器来寻址
            // 之后需要对所有使用到该Value的指令在寄存器分配前要变换。

            // 局部变量偏移设置
            var->setMemoryAddr(ARM64_FP_REG_NO, sp_esp);

            // 累计当前作用域大小
            sp_esp += size;
        }
    }

    // 遍历指令中临时变量
    for (auto inst: func->getInterCode().getInsts()) {

        if (inst->hasResultValue()) {
            // 有值

            int64_t size = inst->getType()->getSize();

            // 64位ARM平台按照8字节的大小整数倍分配局部变量
            size += (8 - size % 8) % 8;

            // 这里要注意检查变量栈的偏移范围。一般采用机制寄存器+立即数方式间接寻址
            // 若立即数满足要求，可采用基址寄存器+立即数变量的方式访问变量
            // 否则，需要先把偏移量放到寄存器中，然后机制寄存器+偏移寄存器来寻址
            // 之后需要对所有使用到该Value的指令在寄存器分配前要变换。

            // 局部变量偏移设置
            inst->setMemoryAddr(ARM64_FP_REG_NO, sp_esp);

            // 累计当前作用域大小
            sp_esp += size;
        }
    }

    // 设置函数的最大栈帧深度，在加上实参内存传值的空间
    // 请注意若支持浮点数，则必须保持栈内空间8字节对齐
    func->setMaxDep(sp_esp);
}
