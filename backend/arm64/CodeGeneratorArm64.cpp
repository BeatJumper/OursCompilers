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
#include "ConstFloat.h"

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
    fprintf(fp, "	%s\n", ".arch armv8-a");
    //代码段
    fprintf(fp, "	%s\n", ".text");
    //代码段按四字节对齐
    fprintf(fp, "	%s\n", ".align 2");
    // 若有浮点运算需求，可添加如下指令支持高级SIMD和浮点单元
    // fpintf(fp, "%s\n", ".fpu neon-fp-armv8");
    //生成的汇编代码将使用 ARM 指令集的指令
    fprintf(fp, "	%s\n", ".cpu generic+fp+simd");

    fprintf(fp, "\n");
}

/// @brief 全局变量Section，主要包含初始化的和未初始化过的
void CodeGeneratorArm64::genDataSection()
{
    printf("genDataSection\n");
    fprintf(fp, "\n");
    // 生成数据段

    bool bssStarted = false;
    bool dataStarted = false;

    // 全局变量分两种情况：初始化的全局变量和未初始化的全局变量
    for (auto var: module->getGlobalVariables()) {
        if (var->isInBSSSection()) {
            // 在BSS段的全局变量
            fprintf(fp, "	.type %s, @object\n", var->getName().c_str());
            if (!bssStarted) {
                fprintf(fp, "	.bss\n");
                bssStarted = true;
            }

            fprintf(fp, "	.global %s\n", var->getName().c_str());
            fprintf(fp, "	.align %d:\n", var->getAlignment());
            fprintf(fp, "%s:\n", var->getName().c_str());
            fprintf(fp, "	.word 0\n");
            fprintf(fp, "	.size %s, %d\n", var->getName().c_str(), var->getType()->getSize());
            //, var->getType()->getSize(), var->getAlignment()
        } else {
            // 有初值的全局变量
            fprintf(fp, "	.type %s, @object\n", var->getName().c_str());
            if (!dataStarted) {
                fprintf(fp, "	.data\n");
                dataStarted = true;
            }

            fprintf(fp, "	.global %s\n", var->getName().c_str());
            fprintf(fp, "	.align %d\n", var->getAlignment());
            fprintf(fp, "%s:\n", var->getName().c_str());

            if (auto constInt = dynamic_cast<ConstInt *>(var->getInitValue())) {
                fprintf(fp, "	.word %d\n", constInt->getVal());
                fprintf(fp, "	.size %s, %d\n", var->getName().c_str(), var->getType()->getSize());
            } else if (auto constFloat = dynamic_cast<ConstFloat *>(var->getInitValue())) {
                uint32_t floatBits;
                float tempFloat = constFloat->getVal();
                std::memcpy(&floatBits, &tempFloat, sizeof(float));
                fprintf(fp, "	.word %u\n", floatBits);
            } /*else if (auto constArray = dynamic_cast<ConstArray *>(var)) {
                // 处理数组类型全局变量
                for (auto element: constArray->getElements()) {
                    if (auto constIntElement = dynamic_cast<ConstInt *>(element)) {
                        fprintf(fp, ".word %d\n", constIntElement->getVal());
                    } else if (auto constFloatElement = dynamic_cast<ConstFloat *>(element)) {
                        uint32_t floatBits;
                        float tempFloatElement = constFloatElement->getVal();
                        std::memcpy(&floatBits, &tempFloatElement, sizeof(float));
                        fprintf(fp, ".word %u\n", floatBits);
                    } else if (auto strElement = dynamic_cast<ConstString *>(element)) {
                        fprintf(fp, ".asciz \"%s\"\n", strElement->getVal().c_str());
                    }
                }
            } else if (auto constStr = dynamic_cast<ConstString *>(var)) {
                // 处理字符串常量
                fprintf(fp, ".asciz \"%s\"\n", constStr->getVal().c_str());
            }*/
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
    fprintf(fp, "	.global %s\n", func->getName().c_str());
    fprintf(fp, "	.type %s, %%function\n", func->getName().c_str());
    fprintf(fp, "	.align %d\n", func->getAlignment());
    fprintf(fp, "%s:\n", func->getName().c_str()); // 直接输出函数名标签
    printf("函数入口标签\n");

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
        printf("输出指令关联的临时变量信息\n");
    }
    iloc.outPut(fp);
    fprintf(fp, "\n");
}

/// @brief 寄存器分配
/// @param func 函数指针
void CodeGeneratorArm64::registerAllocation(Function * func)
{
    // 内置函数不需要处理
    if (func->isBuiltin()) {
        return;
    }

    std::vector<int32_t> & protectedRegNo = func->getProtectedReg();

    protectedRegNo.push_back(ARM64_FP_REG_NO);
    protectedRegNo.push_back(ARM64_LX_REG_NO);
    /*if (func->getExistFuncCall()) {
        protectedRegNo.push_back(ARM64_LX_REG_NO);
    }*/
    printf("寄存器分配中段\n");

    // 调整函数调用指令，主要是前8个寄存器传值，后面用栈传递
    // 为了更好的进行寄存器分配，可以进行对函数调用的指令进行预处理
    // 当然也可以不做处理，不过性能更差。这个处理是可选的。
    // 当前函数的指令列表
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

/// @brief 划分基本块
/// @param func 函数指针
/*void CodeGeneratorArm64::GenBasicBlocks(Function * func)
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
        } else {
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
}*/

/// @brief 寄存器分配前对函数内的指令进行调整，以便方便寄存器分配
/// @param func 要处理的函数
void CodeGeneratorArm64::adjustFormalParamInsts(Function * func)
{
    // 函数形参的前8个实参值临时变量采用的是寄存器传值
    // 前8个之后通过栈传递

    // 请注意这里所得的所有形参都是对应的实参的值关联的临时变量
    // 如果不是不能使用这里的代码
    auto & params = func->getParams();
    printf("params：%d\n", int(params.size()));

    // 形参的前8个通过寄存器来传值X0-X7
    for (int k = 0; k < (int) params.size() && k <= 7; k++) {

        // 前八个设置分配寄存器

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
    // 当前函数的指令列表
    auto & insts = func->getInterCode().getInsts();

    // 函数返回值用x0寄存器，若函数调用有返回值，则赋值x0到对应寄存器
    // 通过栈传递的实参，采用SP + 偏移的方式殉职，偏移肯定非负。
    for (auto pIter = insts.begin(); pIter != insts.end(); pIter++) {

        // 检查是否是函数调用指令，并且含有返回值
        if (Instanceof(callInst, FuncCallInstruction *, *pIter)) {

            // 实参前8个要寄存器传值，其它参数通过栈传递
            Function * f = module->findFunction(callInst->getCalledName());
            int32_t argNum = f->getParams().size();
            printf("初始函数参数个数：%d\n", argNum);

            // 除前8个整数寄存器外，后面的参数采用栈传递
            int esp = 0;
            for (int32_t k = 8; k < argNum; k++) {

                // 获取实参的值
                auto arg = callInst->getOperand(k);

                // 栈帧空间（低地址在前，高地址在后）
                // --------------------- sp
                // 实参栈传递的空间（排除寄存器传递的实参空间）
                // ---------------------
                // 需要保存在栈中的局部变量或临时变量或形参对应变量空间
                // --------------------- fp
                // 保护寄存器的空间
                // ---------------------

                // 新建一个内存变量，把实参的值保存到栈中，以便栈传值，其寻址为SP + 非负偏移
                MemVariable * newVal = func->newMemVariable(IntegerType::getTypeInt());
                newVal->setMemoryAddr(ARM64_SP_REG_NO, esp);
                esp += 8;

                // 引入赋值指令，把实参的值保存到内存变量上
                Instruction * assignInst = new MoveInstruction(func, newVal, arg);

                // 更换实参变量为内存变量
                callInst->setOperand(k, newVal);

                // 赋值指令插入到函数调用指令的前面
                // 函数调用指令前插入后，pIter仍指向函数调用指令
                pIter = insts.insert(pIter, assignInst);
                printf("插入一条赋值指令\n");
                pIter++;
            }

            // ARM64的函数调用约定，前8个参数通过寄存器传递
            int regArgs = std::min(argNum, 8);
            for (int k = 0; k < regArgs; k++) {

                // 把实参的值通过move指令传递给寄存器

                auto arg = callInst->getOperand(k);
                Instruction * assignInst = new MoveInstruction(func, PlatformArm64::intRegVal[k], arg);

                callInst->setOperand(k, PlatformArm64::intRegVal[k]);

                // 函数调用指令前插入后，pIter仍指向函数调用指令
                pIter = insts.insert(pIter, assignInst);
                printf("插入一条赋值指令\n");
                pIter++;
            }

#if 0
            for (int k = 0; k < callInst->getOperandsNum(); k++) {

                auto arg = callInst->getOperand(k);

                // 产生ARG指令
                pIter = insts.insert(pIter, new ArgInstruction(func, arg));
                pIter++;
            }
#endif

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
                    //  函数调用指令的下一个指令的前面插入指令，因为有Exit指令，+1肯定有效
                    pIter = insts.insert(pIter + 1, assignInst);
                    printf("插入一条赋值指令\n");
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

    // 只处理未分配到寄存器的局部变量
    for (auto local: func->getVarValues()) {
        if (local->getRegId() != -1) {
            continue; // 跳过已分配寄存器的变量
        }
        // 对齐到4字节边界
        sp_esp = (sp_esp + 3) & ~3;
        local->setMemoryAddr(ARM64_SP_REG_NO, -sp_esp);
        sp_esp += local->getType()->getSize();
    }

    /*printf("开始处理Alloca\n");
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
        if (inst->getOp() == IRInstOperator::IRINST_OP_FUNC_CALL) {
            // 有值
            int32_t size = inst->getType()->getSize();

            // 64位ARM平台按照8字节的大小整数倍分配局部变量
            size += (8 - size % 8) % 8;

            // 临时变量偏移设置
            inst->setMemoryAddr(ARM64_FP_REG_NO, sp_esp);

            // 累计当前作用域大小
            sp_esp += size;
        }
    }*/

    // 遍历指令中临时变量
    for (auto inst: func->getInterCode().getInsts()) {

        if (inst->hasResultValue()) {
            // 有值
            int32_t size = inst->getType()->getSize();

            // 按照4字节的大小整数倍分配局部变量
            size += (4 - size % 4) % 4;

            // 临时变量偏移设置
            inst->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);

            // 累计当前作用域大小
            sp_esp += size;
        }
    }

    // 设置函数的最大栈帧深度，在加上实参内存传值的空间
    func->setMaxDep(sp_esp);
}
