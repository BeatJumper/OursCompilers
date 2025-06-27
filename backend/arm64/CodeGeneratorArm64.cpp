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
#include "ILocArm64.h"
#include "RegVariable.h"
#include "FuncCallInstruction.h"
#include "ArgInstruction.h"
#include "MoveInstruction.h"
#include "Instruction.h"
#include "ConstFloat.h"
#include "LocalVariable.h"
#include "BinaryInstruction.h"
#include "StoreInstruction.h"
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
    printf("genDataSection\n");
    // 生成数据段
    // TODO全局常量
    bool bssStarted = false;
    bool dataStarted = false;

    // 全局变量分两种情况：初始化的全局变量和未初始化的全局变量
    for (auto var: module->getGlobalVariables()) {
        if (var->isInBSSSection() && var->getInitValueList().empty() && !var->getInitValue()) {
            // 在BSS段的全局变量（没有初始化值）
            fprintf(fp, ".type %s, @object\n", var->getName().c_str());
            if (!bssStarted) {
                fprintf(fp, ".bss\n");
                bssStarted = true;
            }

            fprintf(fp, ".global %s\n", var->getName().c_str());
            fprintf(fp, ".align %d\n", var->getAlignment());
            fprintf(fp, "%s:\n", var->getName().c_str());

            // 对于数组类型，需要分配足够的空间
            if (var->getType()->isArrayType()) {
                int totalSize = var->getType()->getSize();
                int wordCount = (totalSize + 3) / 4; // 向上取整到字边界
                for (int i = 0; i < wordCount; i++) {
                    fprintf(fp, ".word 0\n");
                }
            } else {
                fprintf(fp, ".word 0\n");
            }
            fprintf(fp, ".size %s, %d\n", var->getName().c_str(), var->getType()->getSize());
            //, var->getType()->getSize(), var->getAlignment()
        } else {
            // 有初值的全局变量
            fprintf(fp, ".type %s, @object\n", var->getName().c_str());
            if (!dataStarted) {
                fprintf(fp, ".data\n");
                dataStarted = true;
            }

            fprintf(fp, ".global %s\n", var->getName().c_str());
            fprintf(fp, ".align %d\n", var->getAlignment());
            fprintf(fp, "%s:\n", var->getName().c_str());

            if (auto constInt = dynamic_cast<ConstInt *>(var->getInitValue())) {
                fprintf(fp, ".word %d\n", constInt->getVal());
                fprintf(fp, ".size %s, %d\n", var->getName().c_str(), var->getType()->getSize());
            } else if (auto constFloat = dynamic_cast<ConstFloat *>(var->getInitValue())) {
                uint32_t floatBits;
                float tempFloat = constFloat->getVal();
                std::memcpy(&floatBits, &tempFloat, sizeof(float));
                fprintf(fp, ".word %u\n", floatBits);
            } else if (var->getType()->isArrayType() && !var->getInitValueList().empty()) {
                // 处理数组类型全局变量的初始化值列表
                auto & initValues = var->getInitValueList();
                expandAndOutputInitValues(initValues);
                fprintf(fp, ".size %s, %d\n", var->getName().c_str(), var->getType()->getSize());
            } else {
                // 默认情况：输出单个0
                fprintf(fp, ".word 0\n");
                fprintf(fp, ".size %s, %d\n", var->getName().c_str(), var->getType()->getSize());
            }
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
            inst->setIRName(IR_LABEL_PREFIX + std::to_string(labelIndex++));
        }
    }
    // ILOC代码序列
    ILocArm64 iloc(module);

    // 指令选择生成汇编指令
    InstSelectorArm64 instSelector(IrInsts, iloc, func);
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

    // 函数开始,先释放所有寄存器,强制占用w0-w7寄存器，用于函数调用传递参数
    /*for (int i = 0; i < 29; i++) {
        simpleRegisterAllocator.free(i);
    }
    for (int i = 0; i < 8; i++) {
        simpleRegisterAllocator.Allocate(i);
    }*/

    std::vector<int32_t> & protectedRegNo = func->getProtectedReg();

    protectedRegNo.push_back(ARM64_FP_REG_NO);
    protectedRegNo.push_back(ARM64_LX_REG_NO);
    printf("寄存器分配中段\n");

    // 给STORE用到的立即数添加MOV指令
    adjustMovInsts(func);

    // 调整函数调用指令，主要是前8个寄存器传值，后面用栈传递
    // 为了更好的进行寄存器分配，可以进行对函数调用的指令进行预处理
    // 当然也可以不做处理，不过性能更差。这个处理是可选的。
    // 当前函数的指令列表
    adjustFuncCallInsts(func);
    printf("调整函数调用指令\n");

    adjustBinaryInsts(func);

    // 加完新指令后也该重新调整IR编号
    func->renameIR();

    // 主要染色过程（不断尝试染色直至成功）
    while (true) {
        // 为局部变量和临时变量在栈内分配空间，指定偏移，进行栈空间的分配
        stackAlloc(func);
        printf("为局部变量和临时变量在栈内分配空间\n");

        // 创建干涉图
        InterferenceGraph * graph_ig = new InterferenceGraph(func);

        // 尝试进行染色
        printf("干涉图已产生\n");
        // 染色是否成功
        bool suc = InterferenceGraph::color_graph(graph_ig, PlatformArm64::maxUsableRegNum);

        printf("完成染色\n");
        if (suc) {
            // assert(graph_ig->node_set.size());
            for (node_IG * node: graph_ig->node_set) {
                // assert(node->color != -1);
                // std::cout << InterferenceGraph::ColorToRegId(node->color) << std::endl;
                node->val->setRegId(InterferenceGraph::ColorToRegId(node->color));
                // std::cout << node->val->getRegId() << std::endl;
            }
            break;
        } else {
            // TODO 完成变量溢出的工作
            assert(false);
        }
    }

    // 函数形参要求前8个寄存器分配，后面的参数采用栈传递，实现实参的值传递给形参
    // 这一步是必须的
    adjustFormalParamInsts(func);
    printf("函数形参\n");
    // GenBasicBlocks(func);
    // printf("基本块划分成功\n");

    /*#if 0
        // 临时输出调整后的IR指令，用于查看当前的寄存器分配、栈内变量分配、实参入栈等信息的正确性
        std::string irCodeStr;
        func->toString(irCodeStr);
        std::cout << irCodeStr << std::endl;
    #endif*/
}

/// @brief 寄存器分配前对常数进行扫描，对一些常数提前追加MOV指令
/// @param func
void CodeGeneratorArm64::adjustMovInsts(Function * func)
{
    auto & insts = func->getInterCode().getInsts();
    for (size_t i = 0; i < insts.size();) {

        //目前第i条指令
        Instruction * inst = insts[i];
        //检测是否是Store指令
        if (dynamic_cast<StoreInstruction *>(inst)) {
            //要存入的数
            Value * val = inst->getOperand(0);
            //检测要存入的数是否是constant
            if (Instanceof(const_val, Constant *, val)) {
                // 对于常量0，不需要创建MoveInstruction，ARM64有专门的零寄存器
                ConstInt * constInt = dynamic_cast<ConstInt *>(const_val);
                if (constInt && constInt->getVal() == 0) {
                    // 常量0保持原样，在指令翻译时使用零寄存器
                } else {
                    // 其他常量需要mov到寄存器
                    Value * newval = new Value(val->getType());
                    // 为constant创建mov指令
                    MoveInstruction * movinst = new MoveInstruction(func, newval, val);
                    //指令的对应constant操作数修改为新创建的寄存器变量
                    insts[i]->getOperands()[0] = new Use(newval, inst);

                    //插入到当前位置
                    insts.insert(insts.begin() + i, (Instruction *) movinst);
                    //插入后当前位置变为新插入的指令，故i额外+1
                    i++;
                }
            }
        }
        i++;
    }
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

                // 创建一个新的临时变量来表示寄存器参数，并设置其寄存器ID
                Value * regParam = new Value(arg->getType());
                regParam->setRegId(k);

                // 检查源操作数是否已经在目标寄存器中，避免生成自赋值指令
                if (arg->getRegId() != k) {
                    Instruction * assignInst = new MoveInstruction(func, regParam, arg);

                    // 函数调用指令前插入后，pIter仍指向函数调用指令
                    pIter = insts.insert(pIter, assignInst);
                    printf("插入第%d个参数的赋值指令\n", k);
                    pIter++;
                }

                callInst->setOperand(k, regParam);
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
                    // 创建一个表示 x0 寄存器的临时变量
                    Value * retRegVar = new Value(callInst->getType());
                    retRegVar->setRegId(0);

                    // 新建一个赋值操作
                    Instruction * assignInst = new MoveInstruction(func, callInst, retRegVar);
                    //  函数调用指令的下一个指令的前面插入指令，因为有Exit指令，+1肯定有效
                    pIter = insts.insert(pIter + 1, assignInst);
                    printf("插入一条赋值指令\n");
                }
            }
        }
    }
}

/// @brief 寄存器分配后对乘法和除法操作数里的常量添加赋值
/// @param func 要处理的函数
void CodeGeneratorArm64::adjustBinaryInsts(Function * func)
{
    // 当前函数的指令列表
    auto & insts = func->getInterCode().getInsts();

    // 函数返回值用x0寄存器，若函数调用有返回值，则赋值x0到对应寄存器
    // 通过栈传递的实参，采用SP + 偏移的方式殉职，偏移肯定非负。
    for (auto pIter = insts.begin(); pIter != insts.end(); pIter++) {
        if (Instanceof(binaryInst, BinaryInstruction *, *pIter)) {
            if (binaryInst->getOp() == IRInstOperator::IRINST_OP_MUL_I ||
                binaryInst->getOp() == IRInstOperator::IRINST_OP_DIV_I ||
                binaryInst->getOp() == IRInstOperator::IRINST_OP_MOD_I) {
                printf("检测到两元乘法除法指令\n");
                Value * arg1 = binaryInst->getOperand(0);
                Value * arg2 = binaryInst->getOperand(1);
                if (dynamic_cast<ConstInt *>(arg1)) {
                    printf("检测到操作数1为常量\n");
                    Value * newval = new Value(arg1->getType());
                    Instruction * assignInst = new MoveInstruction(func, newval, arg1);
                    binaryInst->getOperands()[0] = new Use(newval, binaryInst);
                    pIter = insts.insert(pIter, assignInst);
                    printf("插入一条赋值指令\n");
                    pIter++;
                }
                if (dynamic_cast<ConstInt *>(arg2)) {
                    printf("检测到操作数2为常量\n");
                    Value * newval = new Value(arg2->getType());
                    Instruction * assignInst = new MoveInstruction(func, newval, arg2);
                    binaryInst->getOperands()[1] = new Use(newval, binaryInst);
                    pIter = insts.insert(pIter, assignInst);
                    printf("插入一条赋值指令\n");
                    pIter++;
                }
            }
            if (binaryInst->getOp() == IRInstOperator::IRINST_OP_ADD_I ||
                binaryInst->getOp() == IRInstOperator::IRINST_OP_SUB_I) {
                printf("检测到两元加法减法指令\n");
                Value * arg1 = binaryInst->getOperand(0);
                if (dynamic_cast<ConstInt *>(arg1)) {
                    printf("检测到操作数1为常量\n");
                    Value * newval = new Value(arg1->getType());
                    Instruction * assignInst = new MoveInstruction(func, newval, arg1);
                    binaryInst->getOperands()[0] = new Use(newval, binaryInst);
                    pIter = insts.insert(pIter, assignInst);
                    printf("插入一条赋值指令\n");
                    pIter++;
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

        // 检查是否是数组类型的局部变量，如果是则跳过（它们由alloca指令处理）
        bool isArrayVariable = local->getType()->isArrayType();

        if (isArrayVariable) {

            continue; // 跳过数组变量，它们的地址将在alloca处理阶段设置
        }

        // 对齐到4字节边界
        sp_esp = (sp_esp + 3) & ~3;
        local->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);

        sp_esp += local->getType()->getSize();
    }

    // 遍历指令中的alloca指令，为它们分配的数组分配栈空间
    for (auto inst: func->getInterCode().getInsts()) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_ALLOCA) {
            // alloca指令需要为它要分配的数组分配栈空间
            // alloca指令的结果是指向这个数组的指针

            // 简单的大小估算：根据指令类型
            Type * allocatedType = inst->getType();
            int64_t size = 8; // 默认大小

            if (allocatedType) {
                size = allocatedType->getSize();
                if (size == 0) {
                    size = 8;
                }
            }

            // 对于数组类型的alloca，分配更大的空间
            // 这里使用一个简单的启发式：如果大小小于16，设为16
            if (size < 16) {
                size = 16;
            }

            // 16字节对齐（ARM64要求）
            size = (size + 15) & ~15;

            // 为alloca指令设置内存地址，这个地址指向分配的数组空间的起始位置
            // 注意：这里设置的是alloca指令本身的内存地址，
            // 在指令翻译时，lea_var会使用这个地址
            inst->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);

            // 为alloca指令的结果变量设置相同的内存地址
            if (inst->getOperandsNum() > 0) {
                Value * result = inst->getOperand(0);
                // 尝试将结果变量转换为LocalVariable并设置内存地址
                if (LocalVariable * localVar = dynamic_cast<LocalVariable *>(result)) {
                    localVar->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);
                }
            }
            sp_esp += size;
        }
    }

    // 遍历指令中临时变量
    for (auto inst: func->getInterCode().getInsts()) {

        if (inst->hasResultValue() && inst->getOp() != IRInstOperator::IRINST_OP_ALLOCA) {
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

/// @brief 递归展开并输出初始化值列表
/// @param initValues 初始化值列表
void CodeGeneratorArm64::expandAndOutputInitValues(const std::vector<Value *> & initValues)
{
    for (auto element: initValues) {
        if (auto constIntElement = dynamic_cast<ConstInt *>(element)) {
            fprintf(fp, ".word %d\n", constIntElement->getVal());
        } else if (auto constFloatElement = dynamic_cast<ConstFloat *>(element)) {
            uint32_t floatBits;
            float tempFloatElement = constFloatElement->getVal();
            std::memcpy(&floatBits, &tempFloatElement, sizeof(float));
            fprintf(fp, ".word %u\n", floatBits);
        } else if (auto globalVarElement = dynamic_cast<GlobalVariable *>(element)) {
            // 递归处理嵌套的全局变量（嵌套数组）
            if (!globalVarElement->getInitValueList().empty()) {
                expandAndOutputInitValues(globalVarElement->getInitValueList());
            } else {
                // 如果嵌套的全局变量没有初始化值列表，输出0
                fprintf(fp, ".word 0\n");
            }
        } else {
            // 默认输出0
            fprintf(fp, ".word 0\n");
        }
    }
}
