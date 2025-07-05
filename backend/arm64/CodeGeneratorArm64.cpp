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
#include "AllocaInstruction.h"
#include "InterferenceGraph.h"
#include "LoadInstruction.h"
#include "VoidType.h"
#include "PointerType.h"

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
    // fprintf(fp, "%s\n", ".fpu neon-fp-armv8");
    //生成的汇编代码将使用 ARM 指令集的指令
    fprintf(fp, "%s\n", ".cpu generic+fp+simd");

    fprintf(fp, "\n");
}

/// @brief 全局变量Section，主要包含初始化的和未初始化过的
void CodeGeneratorArm64::genDataSection()
{
    printf("genDataSection\n");
    // 生成数据段
    bool bssStarted = false;
    bool dataStarted = false;

    // 全局变量分两种情况：初始化的全局变量和未初始化的全局变量
    for (auto var: module->getGlobalVariables()) {
        printf("Debug: Processing global variable '%s', isInBSSSection=%d, initValueList.size()=%zu, hasInitValue=%d\n",
               var->getName().c_str(),
               var->isInBSSSection(),
               var->getInitValueList().size(),
               var->getInitValue() != nullptr);
        // 检查变量是否真的应该放在BSS段（没有任何初始化值）
        bool shouldBeInBSS = var->isInBSSSection() && var->getInitValueList().empty() && !var->getInitValue();

        if (shouldBeInBSS) {
            // 在BSS段的全局变量（没有初始化值）
            fprintf(fp, ".type %s, @object\n", var->getName().c_str());
            if (!bssStarted) {
                fprintf(fp, ".bss\n");
                bssStarted = true;
                dataStarted = false; // 重置data段标志
            }

            fprintf(fp, ".global %s\n", var->getName().c_str());
            fprintf(fp, ".align %d\n", var->getAlignment());
            fprintf(fp, "%s:\n", var->getName().c_str());

            // 对于数组类型，需要分配足够的空间
            // 使用存储类型来获取正确的大小信息
            Type * actualType = var->getStorageType() ? var->getStorageType() : var->getType();

            if (actualType->isArrayType()) {
                printf("Debug: BSS段数组 %s，实际类型: %s\n", var->getName().c_str(), actualType->toString().c_str());
                int totalSize = actualType->getSize();
                int wordCount = (totalSize + 3) / 4; // 向上取整到字边界
                printf("Debug: 总大小: %d字节，需要 %d 个word\n", totalSize, wordCount);

                // 对于大数组，使用.space指令而不是逐个.word 0
                if (wordCount > 1000) {
                    fprintf(fp, "	.space %d\n", totalSize);
                    printf("Debug: BSS段使用.space指令分配 %d 字节的零初始化内存\n", totalSize);
                } else {
                    for (int i = 0; i < wordCount; i++) {
                        fprintf(fp, "	.word 0\n");
                    }
                }
                fprintf(fp, ".size %s, %d\n", var->getName().c_str(), totalSize);
            } else {
                fprintf(fp, "	.word 0\n");
                fprintf(fp, ".size %s, %d\n", var->getName().c_str(), actualType->getSize());
            }
            //, var->getType()->getSize(), var->getAlignment()
        } else {
            // 有初值的全局变量（应该放在data段）
            fprintf(fp, ".type %s, @object\n", var->getName().c_str());
            if (!dataStarted) {
                fprintf(fp, ".data\n");
                dataStarted = true;
                bssStarted = false; // 重置bss段标志
            }

            fprintf(fp, ".global %s\n", var->getName().c_str());
            fprintf(fp, ".align %d\n", var->getAlignment());
            fprintf(fp, "%s:\n", var->getName().c_str());

            if (auto constInt = dynamic_cast<ConstInt *>(var->getInitValue())) {
                fprintf(fp, "	.word %d\n", constInt->getVal());
                fprintf(fp, ".size %s, %d\n", var->getName().c_str(), var->getType()->getSize());
            } else if (auto constFloat = dynamic_cast<ConstFloat *>(var->getInitValue())) {
                uint32_t floatBits;
                float tempFloat = constFloat->getVal();
                std::memcpy(&floatBits, &tempFloat, sizeof(float));
                fprintf(fp, "	.word %u\n", floatBits);
            } else {
                // 检查存储类型是否是数组类型，或者直接检查初始化值列表
                Type * storageType = var->getStorageType();
                bool isArrayType = (storageType && storageType->isArrayType()) || var->getType()->isArrayType();

                if (isArrayType || !var->getInitValueList().empty()) {
                    // 处理数组类型全局变量的初始化值列表
                    printf("处理数组类型全局变量的初始化值列表\n");
                    if (!var->getInitValueList().empty()) {
                        printf("数组初始化值列表非空\n");
                        auto & initValues = var->getInitValueList();
                        expandAndOutputInitValues(initValues);

                        // 计算实际输出的字节数：展开后的元素个数 * 4字节
                        int actualElements = countExpandedInitValues(initValues);
                        int actualSize = actualElements * 4;
                        fprintf(fp, ".size %s, %d\n", var->getName().c_str(), actualSize);
                    } else if (var->getInitValue()) {
                        // 单个值情况
                        int size = 0;
                        if (ConstInt * value = dynamic_cast<ConstInt *>(var->getInitValue())) {
                            size = 4;
                            fprintf(fp, "	.word %d\n", value->getVal());
                        } else if (ConstFloat * value = dynamic_cast<ConstFloat *>(var->getInitValue())) {
                            size = 4;
                            fprintf(fp, "	.word %f\n", value->getVal());
                        }
                        fprintf(fp, ".size %s, %d\n", var->getName().c_str(), size);
                    } else {
                        // zeroinitializer处理
                        if (storageType && storageType->isArrayType()) {
                            ArrayType * arrayType = static_cast<ArrayType *>(storageType);
                            int totalElements = arrayType->getTotalElements();
                            // 对于大数组，使用.space指令而不是逐个.word 0
                            if (totalElements > 1000) {
                                int totalBytes = totalElements * 4; // 每个元素4字节
                                fprintf(fp, "	.space %d\n", totalBytes);
                                printf("Debug: 使用.space指令分配 %d 字节的零初始化内存\n", totalBytes);
                            } else {
                                for (int i = 0; i < totalElements; i++) {
                                    fprintf(fp, "	.word 0\n");
                                }
                            }
                        } else if (var->getType()->isArrayType()) {
                            ArrayType * arrayType = static_cast<ArrayType *>(var->getType());
                            int totalElements = arrayType->getTotalElements();
                            // 对于大数组，使用.space指令而不是逐个.word 0
                            if (totalElements > 1000) {
                                int totalBytes = totalElements * 4; // 每个元素4字节
                                fprintf(fp, "	.space %d\n", totalBytes);
                                printf("Debug: 使用.space指令分配 %d 字节的零初始化内存\n", totalBytes);
                            } else {
                                for (int i = 0; i < totalElements; i++) {
                                    fprintf(fp, "	.word 0\n");
                                }
                            }
                        }
                    }
                } else {
                    // 非数组类型的其他情况
                    fprintf(fp, "	.word 0\n");
                    fprintf(fp, ".size %s, %d\n", var->getName().c_str(), var->getType()->getSize());
                }
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
    // 获取函数的指令列表
    std::vector<Instruction *> & IrInsts = func->getInterCode().getInsts();
    printf("成功获取指令列表，指令数量：%d\n", int(IrInsts.size()));

    // 标签已经在前端IR生成时确保全局唯一，无需重新编号
    // ILOC代码序列
    ILocArm64 iloc(module);
    // 寄存器分配以及栈空间分配
    registerAllocation(func);
    printf("寄存器分配完成\n");

    iloc.allocStack(func, ARM64_TMP_REG_NO);

    // 指令选择生成汇编指令
    InstSelectorArm64 instSelector(IrInsts, iloc, func);
    instSelector.setShowLinearIR(this->showLinearIR);
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

void CodeGeneratorArm64::spill(Function * func, InterferenceGraph * graph_ig)
{
    // 首先取出目前干涉图中度数最高的Value（未染色的）
    node_IG * most_degree_node = nullptr;
    // assert(graph_ig->uncolored_node_set.size());
    for (node_IG * node: graph_ig->uncolored_node_set) {
        if (node->val->getRegId() != -1) {
            continue;
        }
        if (most_degree_node == nullptr || most_degree_node->degree() < node->degree()) {
            most_degree_node = node;
        }
    }
    if (most_degree_node == nullptr) {
        return;
    }
    std::cout << "most degree = " << most_degree_node->degree() << std::endl;
    printval(most_degree_node->val);
    printf("neighbors:\n");
    for (node_IG * neighbor: most_degree_node->neighbors) {
        printval(neighbor->val);
    }

    // 为溢出该变量分配的栈空间
    LocalVariable * localval = nullptr;

    Value * most_degree_val = most_degree_node->val;
    // int32_t reg_now = most_degree_val->getRegId();
    auto & insts = func->getInterCode().getInsts();

    // 接下来尝试把该Value的所有出现都替换为新的Value和LocalVariable
    for (int i = 0; i < insts.size(); i++) {
        // Value * regval_read = nullptr;

        if (insts[i]->get_def_set().count(most_degree_val)) {
            // DEF是其中某一个操作数的情况
            if (localval == nullptr) {
                localval = func->newLocalVarValue(most_degree_val->getType());
            }
            Instruction * strinst = new StoreInstruction(func, most_degree_val, localval);
            insts.insert(insts.begin() + i + 1, strinst);
        }
        if (insts[i]->get_use_set().count(most_degree_val)) {
            if (localval == nullptr) {
                localval = func->newLocalVarValue(most_degree_val->getType());
            }
            // regval_read = new Value(most_degree_val->getType());
            // regval_read->setRegId(reg_now);
            Instruction * ldrinst = new LoadInstruction(func, nullptr, localval);
            insts.insert(insts.begin() + i, ldrinst);
            i++;
            for (int k = 0; k < insts[i]->getOperandsNum(); k++) {
                if (insts[i]->getOperand(k) == most_degree_val) {
                    insts[i]->getOperands()[k]->setUsee(ldrinst);
                }
            }
        }
    }
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

    // 调整函数调用指令，主要是前8个寄存器传值，后面用栈传递
    // 为了更好的进行寄存器分配，可以进行对函数调用的指令进行预处理
    // 当然也可以不做处理，不过性能更差。这个处理是可选的。
    // 当前函数的指令列表
    adjustFuncCallInsts(func);
    printf("调整函数调用指令\n");

    adjustFormalParamInsts(func);

    // 给一些指令添加临时调整指令
    adjustSomeInsts(func);

    // 目前renameIR在创建干涉图阶段产生，因为renameIR为了能给adjust新建的IR命名需要DEF和USE集
    // 因此需要调用Instruction类的transfer()来更新DEF和USE集
    /*
    // 加完新指令后也该重新调整IR编号
    func->renameIR();
    */

    // 主要染色过程（不断尝试染色直至成功）
    bool forfloat[] = {false, true};
    for (bool is_float: forfloat) {
        while (true) {
            // 创建干涉图
            // InterferenceGraph * graph_ig1 = new InterferenceGraph(func, is_float);

            // spill(func, graph_ig1);

            InterferenceGraph * graph_ig = new InterferenceGraph(func, is_float);

            // spill(func, graph_ig);

            std::cout << "完成干涉图构建" << std::endl;

            // 尝试进行染色
            // 染色是否成功
            bool suc =
                InterferenceGraph::color_graph(graph_ig,
                                               is_float ? PlatformArm64::maxVecRegNum : PlatformArm64::maxUsableRegNum);

            if (suc) {
                // assert(graph_ig->node_set.size());
                for (node_IG * node: graph_ig->node_set) {
                    // assert(node->color != -1);
                    // std::cout << InterferenceGraph::ColorToRegId(node->color) << std::endl;
                    // printval(func->getParams()[0]);
                    // printval(node->val);
                    node->val->setRegId(
                        InterferenceGraph::ColorToRegId(node->color,
                                                        node->val->getType() == FloatType::getTypeFloat()));

                    // std::cout << node->val->getRegId() << std::endl;
                }
                // std::cout << "染色成功" << std::endl;
                break;
            } else {
                // assert(false);
                spill(func, graph_ig);
                // 完成变量溢出的工作
            }
        }
    }

    // 为局部变量、数组、返回值、保护寄存器分配栈空间
    stackAlloc(func);

    // 这里加一个set临时存储保护寄存器，因为同一个寄存器可能多次加入，这里用set可以去重。
    std::set<int32_t> protectedreg_set;
    // 如果用到了保护寄存器，就加进保护寄存器集合
    for (Instruction * inst: func->getInterCode().getCode()) {
        for (Value * val: inst->get_use_set()) {
            // 这里为什么有val->getRegId() < 32？因为要考虑到本系统目前给浮点寄存器分配了大于31的regID
            if (val->getRegId() >= 19 && val->getRegId() < 32) {
                protectedreg_set.insert(val->getRegId());
            }
        }
        for (Value * val: inst->get_def_set()) {
            if (val->getRegId() > 18 && val->getRegId() < 32) {
                protectedreg_set.insert(val->getRegId());
            }
        }
    }
    // 然后把集合元素加进真正要用的链表里
    for (int32_t reg: protectedreg_set) {
        protectedRegNo.push_back(reg);
    }

    // 保护寄存器的内存分配见ILocArm64::allocStack和ILocArm64::emitFunctionEpilogue处改动

    printf("为局部变量和临时变量在栈内分配空间\n");
}

/// @brief 寄存器分配前对常数进行扫描，对一些常数提前追加MOV指令
/// @param func
void CodeGeneratorArm64::adjustSomeInsts(Function * func)
{
    auto & insts = func->getInterCode().getInsts();
    for (size_t i = 0; i < insts.size();) {

        //目前第i条指令
        Instruction * inst = insts[i];
        //检测是否是Store指令
        if (dynamic_cast<StoreInstruction *>(inst)) {
            //要存入的数
            Value * val1 = inst->getOperand(0);
            //检测要存入的数是否是constant
            if (Instanceof(const_val, ConstInt *, val1)) {
                // 对于常量0，不需要创建MoveInstruction，ARM64有专门的零寄存器
                if (const_val->getVal() != 0) {
                    // 其他常量需要mov到寄存器
                    Value * newval = new Value(val1->getType());
                    // 为constant创建mov指令
                    MoveInstruction * movinst = new MoveInstruction(func, newval, val1);
                    //指令的对应constant操作数修改为新创建的寄存器变量
                    insts[i]->getOperands()[0] = new Use(newval, inst);

                    //插入到当前位置
                    insts.insert(insts.begin() + i, (Instruction *) movinst);
                    //插入后当前位置变为新插入的指令，故i额外+1
                    i++;
                }
            }
        }
        if (Instanceof(binaryInst, BinaryInstruction *, inst)) {
            printf("检测到两元指令\n");
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
                    insts.insert(insts.begin() + i, (Instruction *) assignInst);
                    printf("插入一条赋值指令\n");
                    //插入后当前位置变为新插入的指令，故i额外+1
                    i++;
                }
                if (dynamic_cast<ConstInt *>(arg2)) {
                    printf("检测到操作数2为常量\n");
                    Value * newval = new Value(arg2->getType());
                    Instruction * assignInst = new MoveInstruction(func, newval, arg2);
                    binaryInst->getOperands()[1] = new Use(newval, binaryInst);
                    insts.insert(insts.begin() + i, (Instruction *) assignInst);
                    printf("插入一条赋值指令\n");
                    i++;
                }
            } else if (binaryInst->getOp() == IRInstOperator::IRINST_OP_ADD_I ||
                       binaryInst->getOp() == IRInstOperator::IRINST_OP_SUB_I) {
                printf("检测到两元加法减法指令\n");
                Value * arg1 = binaryInst->getOperand(0);
                if (dynamic_cast<ConstInt *>(arg1)) {
                    printf("检测到操作数1为常量\n");
                    Value * newval = new Value(arg1->getType());
                    Instruction * assignInst = new MoveInstruction(func, newval, arg1);
                    binaryInst->getOperands()[0] = new Use(newval, binaryInst);
                    insts.insert(insts.begin() + i, (Instruction *) assignInst);
                    printf("插入一条赋值指令\n");
                    i++;
                }
            }
        }
        if (inst->getOp() == IRInstOperator::IRINST_OP_MOD_I) {
            // 对于mod指令，将其转化成乘法除法和减法指令
            // a % b => a - (a / b) * b
            printf("检测到取余指令\n");
            Value * arg1 = inst->getOperand(0);
            Value * arg2 = inst->getOperand(1);

            BinaryInstruction * divInst =
                new BinaryInstruction(func, IRInstOperator::IRINST_OP_DIV_I, arg1, arg2, arg1->getType());
            BinaryInstruction * mulInst =
                new BinaryInstruction(func, IRInstOperator::IRINST_OP_MUL_I, divInst, arg2, arg1->getType());
            BinaryInstruction * subInst =
                new BinaryInstruction(func, IRInstOperator::IRINST_OP_SUB_I, arg1, mulInst, arg1->getType());

            // 关键修复：在删除原指令之前，先替换所有对原指令的引用
            printf("替换所有对mod指令结果的引用\n");
            inst->replaceAllUsesWith(subInst);

            // 删除原来的mod指令
            printf("删除原来的mod指令\n");
            insts.erase(insts.begin() + i);
            // 替换成如下的指令序列
            printf("替换成如下的指令序列\n");
            insts.insert(insts.begin() + i, (Instruction *) divInst);
            i++;
            insts.insert(insts.begin() + i, (Instruction *) mulInst);
            i++;
            insts.insert(insts.begin() + i, (Instruction *) subInst);
            i++;
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

    auto & insts = func->getInterCode().getInsts();
    // 根据ARM版C语言的调用约定，除前8个外的实参进行值传递，逆序入栈
    int64_t maxOffset = func->getMaxDep();
    int64_t fp_esp = maxOffset;
    int protectedRegNum = 0;
    // 保存寄存器空间
    if (func->getExistFuncCall()) {
        protectedRegNum = func->getProtectedReg().size();
        fp_esp += protectedRegNum * 8;
    }
    printf("Debug:被调函数传参前检测栈帧大小:%d\n", int(fp_esp));
    for (int k = 8; k < (int) params.size(); k++) {

        // 第9个及之后的参数位于调用者栈帧中
        // 计算相对于调用者栈帧的偏移：第k个参数的偏移 = (k-8) * 8
        int64_t caller_stack_offset = (k - 8) * 8;

        printf("Debug:第%d个形参位于调用者栈帧偏移:%d\n", k, int(caller_stack_offset));

        // 设置特殊标记，表示这是调用者栈帧中的参数
        // 我们使用负的基址寄存器编号来标记这种特殊情况
        params[k]->setMemoryAddr(-ARM64_SP_REG_NO, caller_stack_offset);

        // 不需要增加fp_esp，因为这些参数不占用当前函数的栈空间

        // 插入ldr指令
        // 这里创建的resVal仅用来翻译load时获取结果的类型
        FormalParam * resVal = new FormalParam(params[k]->getType(), params[k]->getName());
        LoadInstruction * ldrinst = new LoadInstruction(func, resVal, params[k]);

        ldrinst->setRegId(k);
        params[k]->setRegId(k);
        // 把原来引用形参的地方替换为ldrinst的引用
        // params[k]->replaceAllUsesWith(ldrinst);
        insts.insert(insts.begin(), ldrinst);
        if (FormalParam * val = dynamic_cast<FormalParam *>(ldrinst->getOperand(0))) {
            printf("Debug:形参判断逻辑正常\n");
        }
    }
}

/// @brief 寄存器分配前对函数内的指令进行调整，以便方便寄存器分配
/// @param func 要处理的函数
void CodeGeneratorArm64::adjustFuncCallInsts(Function * func)
{
    // 当前函数的指令列表
    auto & insts = func->getInterCode().getInsts();

    // 函数返回值用x0寄存器，若函数调用有返回值，则赋值x0到对应寄存器
    // 通过栈传递的实参，采用SP + 偏移的方式寻址，偏移肯定非负。
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
                printf("检测到8个以后的函数参数\n");

                // 获取实参的值
                auto * arg = callInst->getOperand(k);
                // 新建一个内存变量，把实参的值保存到栈中，以便栈传值，其寻址为SP + 非负偏移

                MemVariable * newVal = func->newMemVariable(arg->getType());
                newVal->setMemoryAddr(ARM64_SP_REG_NO, esp);
                esp += 8;

                // 引入赋值指令，把实参的值保存到内存变量上
                Instruction * assignInst = new StoreInstruction(func, arg, newVal);

                // 更换实参变量为内存变量
                callInst->setOperand(k, newVal);

                // 赋值指令插入到函数调用指令的前面
                // 函数调用指令前插入后，pIter仍指向函数调用指令
                pIter = insts.insert(pIter, assignInst);
                printf("插入一条Store指令（给函数调用的第8个以后的参数）\n");
                pIter++;
            }

            // ARM64的函数调用约定，前8个参数通过寄存器传递
            int regArgs = std::min(argNum, 8);
            for (int k = 0; k < regArgs; k++) {

                // 把实参的值通过move指令传递给寄存器

                auto arg = callInst->getOperand(k);

                // 创建一个新的临时变量来表示寄存器参数，并设置其寄存器ID
                Value * regParam = new Value(arg->getType());

                // 统一使用0-7的寄存器ID，在汇编生成时根据类型选择正确的寄存器名
                regParam->setRegId(k);

                // 检查源操作数是否已经在目标寄存器中，避免生成自赋值指令
                if (arg->getRegId() != regParam->getRegId()) {
                    Instruction * assignInst = new MoveInstruction(func, regParam, arg);

                    // 函数调用指令前插入后，pIter仍指向函数调用指令
                    pIter = insts.insert(pIter, assignInst);
                    printf("为函数调用插入第%d个参数的赋值指令\n", k);
                    pIter++;

                    callInst->setOperand(k, regParam);
                } else {
                    // 源操作数已经在目标寄存器中，直接使用原操作数
                    printf("第%d个参数已在目标寄存器中，跳过赋值指令\n", k);
                    callInst->setOperand(k, arg);
                }
            }

            func->setMaxDep(esp);
            // 有arg指令后可不用参数，展示不删除
            // args.clear();
        }
    }
}

/// @brief 栈空间分配
/// @param func 要处理的函数
void CodeGeneratorArm64::stackAlloc(Function * func)
{
    // 遍历函数内的所有指令，查找没有寄存器分配的变量，然后进行栈内空间分配

    // 这里对临时变量和局部变量都在栈上进行分配,但形参对应实参的临时变量(FormalParam类型)不需要考虑

    int64_t sp_esp = func->getMaxDep();
    printf("stackAlloc开始时,已建立的栈空间大小:%d\n", int(sp_esp));

    // 为数组分配栈空间
    for (auto inst: func->getInterCode().getInsts()) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_ALLOCA) {
            // alloca指令需要为它要分配的数组分配栈空间
            // alloca指令的结果是指向这个数组的指针
            Value * result = inst->getOperand(0);
            // 获取alloca指令分配的类型
            auto * allocatedType = inst->getOperand(0)->getType();
            int64_t size = 4; // 默认大小
            if (Instanceof(arr, ArrayType *, allocatedType)) {
                // alloca对象为数组
                printf("局部变量数组首地址:%d\n", int(sp_esp));
                inst->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);
                // 为alloca指令的结果变量设置相同的内存地址
                // 尝试将结果变量转换为LocalVariable并设置内存地址
                if (LocalVariable * localVar = dynamic_cast<LocalVariable *>(result)) {
                    localVar->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);
                    printf("Debug: stackAlloc - 局部变量 %s 设置内存地址: offset=%ld\n",
                           localVar->getName().c_str(),
                           sp_esp);
                }
                // 使用ArrayType的getSize()方法，它会正确计算所有维度的总大小
                size = arr->getSize();
                printf("Debug: stackAlloc - 数组 %s 类型: %s, 总大小: %ld 字节\n",
                       result->getName().c_str(),
                       arr->toString().c_str(),
                       size);
                sp_esp += size;
            } else if (Instanceof(val, PointerType *, allocatedType)) {
                printf("检测到Alloca对象为指针类型\n");
                auto * pointeeType = val->getPointeeType();
                size = pointeeType->getSize();
                LocalVariable * localVar = dynamic_cast<LocalVariable *>(result);
                localVar->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);
                printf("Alloca指向的数据类型的大小:%d\n", int(size));
                sp_esp += size;
            }
        }
    }

    // 只处理未分配到寄存器的局部变量
    for (auto local: func->getVarValues()) {
        int64_t offset;
        if (local->getMemoryAddr(nullptr, &offset)) {
            continue; // 跳过已分配内存的变量
        }

        // 检查是否是数组类型的局部变量，如果是则跳过（它们由alloca指令处理）
        bool isArrayVariable = local->getType()->isArrayType();

        if (isArrayVariable) {
            continue; // 跳过数组变量，它们的地址将在alloca处理阶段设置
        }

        local->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);

        sp_esp += 4; // local->getType()->getSize()
    }

    // 为所有溢出的临时变量分配栈空间
    printf("Debug: 开始为溢出变量分配栈空间\n");
    int inst_count = 0;
    for (auto inst: func->getInterCode().getInsts()) {
        inst_count++;
        printf("Debug: 检查指令 %d, def_set大小: %zu\n", inst_count, inst->get_def_set().size());

        // 检查指令的定义集合中的变量
        for (Value * val: inst->get_def_set()) {
            printf("Debug: 检查变量 %s, regId=%d\n", val->getIRName().c_str(), val->getRegId());

            // 跳过alloca指令
            if (dynamic_cast<AllocaInstruction *>(val)) {
                printf("Debug: 跳过alloca指令: %s\n", val->getIRName().c_str());
                continue;
            }

            // 只处理溢出的变量（regId=-2）且还没有分配内存地址的变量
            if (val->getRegId() == -2) {
                printf("Debug: 发现溢出变量: %s\n", val->getIRName().c_str());
                int64_t offset;
                if (!val->getMemoryAddr(nullptr, &offset)) {
                    printf("Debug: 变量 %s 没有内存地址，准备分配\n", val->getIRName().c_str());
                    // 为溢出变量分配栈空间
                    if (Instruction * instVal = dynamic_cast<Instruction *>(val)) {
                        instVal->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);
                        printf("为溢出变量 %s 分配栈空间: offset=%ld\n", val->getIRName().c_str(), sp_esp);
                        sp_esp += 4; // 假设都是4字节的整数
                    } else {
                        printf("Debug: 变量 %s 不是Instruction类型\n", val->getIRName().c_str());
                    }
                } else {
                    printf("Debug: 变量 %s 已有内存地址: offset=%ld\n", val->getIRName().c_str(), offset);
                }
            }
        }
    }
    printf("Debug: 溢出变量分配完成，最终栈空间大小: %ld, 总共检查了 %d 条指令\n", sp_esp, inst_count);

    // 返回值占用的栈空间
    Value * returnVal = func->getReturnValue();
    if (LocalVariable * localVar = dynamic_cast<LocalVariable *>(returnVal)) {
        localVar->setMemoryAddr(ARM64_SP_REG_NO, sp_esp);
        sp_esp += 4;
    }

    // 保护寄存器占用的栈空间在生成函数序言中计算

    //栈空间16字节对齐
    sp_esp = (sp_esp + 15) & ~15;

    // 设置函数的最大栈帧深度
    func->setMaxDep(sp_esp);
}

/// @brief 递归展开并输出初始化值列表
/// @param initValues 初始化值列表
void CodeGeneratorArm64::expandAndOutputInitValues(const std::vector<Value *> & initValues)
{
    for (auto element: initValues) {
        if (auto constIntElement = dynamic_cast<ConstInt *>(element)) {
            printf("检测到整数元素\n");
            fprintf(fp, "	.word %d\n", constIntElement->getVal());
        } else if (auto constFloatElement = dynamic_cast<ConstFloat *>(element)) {
            uint32_t floatBits;
            float tempFloatElement = constFloatElement->getVal();
            std::memcpy(&floatBits, &tempFloatElement, sizeof(float));
            fprintf(fp, "	.word %u\n", floatBits);
        } else if (auto globalVarElement = dynamic_cast<GlobalVariable *>(element)) {
            // 递归处理嵌套的全局变量（嵌套数组）
            printf("检测到嵌套数组元素\n");
            if (!globalVarElement->getInitValueList().empty()) {
                expandAndOutputInitValues(globalVarElement->getInitValueList());
            } else {
                // 如果嵌套数组没有初始化值, 对应zeroinitializer处理
                if (globalVarElement->getType()->isArrayType()) {
                    ArrayType * nestedArrayType = static_cast<ArrayType *>(globalVarElement->getType());

                    printf("Debug: Element type: %s\n", nestedArrayType->getElementType()->toString().c_str());

                    int totalElements = nestedArrayType->getTotalElements();
                    printf("Debug: zeroinitializer对应的数组元素个数:%d\n", totalElements);

                    // 对于大数组，使用.space指令而不是逐个.word 0
                    if (totalElements > 1000) {
                        // 使用.space指令分配大块零初始化内存
                        int totalBytes = totalElements * 4; // 每个元素4字节
                        fprintf(fp, "	.space %d\n", totalBytes);
                        printf("Debug: 使用.space指令分配 %d 字节的零初始化内存\n", totalBytes);
                    } else {
                        // 对于小数组，仍然使用.word 0
                        for (int i = 0; i < totalElements; i++) {
                            fprintf(fp, "	.word 0\n");
                        }
                    }
                }
            }
        } else {
            // 默认输出0
            fprintf(fp, "	.word 0\n");
        }
    }
}

/// @brief 计算初始化值列表的实际元素个数（递归展开）
/// @param initValues 初始化值列表
/// @return 实际元素个数
int CodeGeneratorArm64::countExpandedInitValues(const std::vector<Value *> & initValues)
{
    int count = 0;
    for (auto element: initValues) {
        if (auto constIntElement = dynamic_cast<ConstInt *>(element)) {
            count++;
        } else if (auto constFloatElement = dynamic_cast<ConstFloat *>(element)) {
            count++;
        } else if (auto globalVarElement = dynamic_cast<GlobalVariable *>(element)) {
            // 递归计算嵌套的全局变量（嵌套数组）
            if (!globalVarElement->getInitValueList().empty()) {
                count += countExpandedInitValues(globalVarElement->getInitValueList());
            } else {
                // 如果嵌套数组没有初始化值，计算其应有的元素个数
                if (globalVarElement->getType()->isArrayType()) {
                    ArrayType * nestedArrayType = static_cast<ArrayType *>(globalVarElement->getType());
                    count += nestedArrayType->getTotalElements();
                } else {
                    count++;
                }
            }
        } else {
            count++;
        }
    }
    return count;
}