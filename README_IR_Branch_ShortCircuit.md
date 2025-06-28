# IR分支结构与短路求值实现详解

## 1. 概述

本编译器的IR（中间表示）模块实现了完整的分支控制结构和短路求值机制。通过条件跳转指令、标签管理和智能的条件表达式处理，实现了高效的控制流生成，完全符合LLVM IR的语义规范。

## 2. 核心组件架构

### 2.1 主要指令类型

**分支控制相关指令：**
- **`BranchInstruction`** - 条件跳转指令（br i1 %cond, label %true, label %false）
- **`GotoInstruction`** - 无条件跳转指令（br label %target）
- **`LabelInstruction`** - 标签指令（定义跳转目标）

**相关文件结构：**
```
ir/Instructions/
├── BranchInstruction.h/cpp    # 条件跳转指令
├── GotoInstruction.h/cpp      # 无条件跳转指令
└── LabelInstruction.h/cpp     # 标签指令
```

### 2.2 IR生成器中的分支处理

**核心处理函数：**
- `ir_if_else()` - if-else语句的IR生成
- `ir_while()` - while循环的IR生成
- `ir_condition_expr()` - 条件表达式的统一处理
- `ir_and_with_labels()` - 逻辑与的短路求值
- `ir_or_with_labels()` - 逻辑或的短路求值
- `ir_not_with_labels()` - 逻辑非的条件跳转

## 3. 分支结构实现

### 3.1 if-else语句实现

#### 3.1.1 基本结构

```cpp
bool IRGenerator::ir_if_else(ast_node * node)
{
    ast_node * condNode = node->sons[0];     // 条件表达式
    ast_node * thenNode = node->sons[1];     // then分支
    ast_node * elseNode = (node->sons.size() == 3) ? node->sons[2] : nullptr; // else分支（可选）
    
    Function * currentFunc = module->getCurrentFunction();
    
    // 创建标签
    LabelInstruction * thenLabel = new LabelInstruction(currentFunc, module);
    LabelInstruction * elseLabel = elseNode ? new LabelInstruction(currentFunc, module) : nullptr;
    LabelInstruction * endLabel = new LabelInstruction(currentFunc, module);
    
    // 生成条件表达式的IR
    if (!ir_condition_expr(condNode, thenLabel, elseLabel ? elseLabel : endLabel)) {
        return false;
    }
    node->blockInsts.addInst(condNode->blockInsts);
    
    // then分支
    node->blockInsts.addInst(thenLabel);
    if (!ir_visit_ast_node(thenNode)) {
        return false;
    }
    node->blockInsts.addInst(thenNode->blockInsts);
    
    // 检查then分支是否有终结指令
    bool thenHasTerminator = hasTerminatorInstruction(thenNode->blockInsts);
    if (!thenHasTerminator) {
        node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));
    }
    
    // else分支（如果存在）
    if (elseNode) {
        node->blockInsts.addInst(elseLabel);
        if (!ir_visit_ast_node(elseNode)) {
            return false;
        }
        node->blockInsts.addInst(elseNode->blockInsts);
        
        bool elseHasTerminator = hasTerminatorInstruction(elseNode->blockInsts);
        if (!elseHasTerminator) {
            node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));
        }
    }
    
    // 结束标签
    node->blockInsts.addInst(endLabel);
    
    return true;
}
```

#### 3.1.2 生成的IR结构

**C代码示例：**
```c
if (a > b) {
    x = 1;
} else {
    x = 2;
}
```

**生成的IR结构：**
```llvm
; 条件表达式求值
%1 = icmp sgt i32 %a, %b
br i1 %1, label %2, label %3

; then分支
2:
store i32 1, i32* %x
br label %4

; else分支  
3:
store i32 2, i32* %x
br label %4

; 结束标签
4:
; 后续代码
```

### 3.2 while循环实现

#### 3.2.1 循环结构设计

```cpp
bool IRGenerator::ir_while(ast_node * node)
{
    ast_node * condNode = node->sons[0]; // 条件表达式
    ast_node * bodyNode = node->sons[1]; // 循环体
    
    Function * currentFunc = module->getCurrentFunction();
    
    // 创建循环的标签
    LabelInstruction * entryLabel = new LabelInstruction(currentFunc, module);
    LabelInstruction * condLabel = new LabelInstruction(currentFunc, module);
    LabelInstruction * bodyLabel = new LabelInstruction(currentFunc, module);
    LabelInstruction * exitLabel = new LabelInstruction(currentFunc, module);
    
    // 条件检查标签和退出标签压栈（用于break/continue）
    loopLabelStack.push({condLabel, exitLabel});
    
    // 添加入口标签
    node->blockInsts.addInst(entryLabel);
    
    // 跳转到条件检查
    node->blockInsts.addInst(new GotoInstruction(currentFunc, condLabel));
    
    // 条件检查标签
    node->blockInsts.addInst(condLabel);
    
    // 生成条件表达式的IR
    if (!ir_condition_expr(condNode, bodyLabel, exitLabel)) {
        return false;
    }
    node->blockInsts.addInst(condNode->blockInsts);
    
    // 循环体标签
    node->blockInsts.addInst(bodyLabel);
    
    // 生成循环体的IR
    if (!ir_visit_ast_node(bodyNode)) {
        return false;
    }
    node->blockInsts.addInst(bodyNode->blockInsts);
    
    // 跳转回条件检查
    node->blockInsts.addInst(new GotoInstruction(currentFunc, condLabel));
    
    // 退出标签
    node->blockInsts.addInst(exitLabel);
    
    // 弹出循环标签栈
    loopLabelStack.pop();
    
    return true;
}
```

#### 3.2.2 break和continue支持

```cpp
bool IRGenerator::ir_break(ast_node * node)
{
    if (loopLabelStack.empty()) {
        printf("Error: break statement not inside a loop.\n");
        return false;
    }
    
    // 获取当前循环的退出标签
    LabelInstruction * exitLabel = loopLabelStack.top().exitLabel;
    
    // 生成跳转到退出标签的指令
    node->blockInsts.addInst(new GotoInstruction(module->getCurrentFunction(), exitLabel));
    
    return true;
}
```

## 4. 短路求值实现

### 4.1 设计原理

短路求值是指在逻辑表达式中，如果前面的条件已经能够确定整个表达式的结果，就不再计算后面的条件。本编译器通过标签跳转机制实现了完整的短路求值。

### 4.2 逻辑与（&&）的短路求值

#### 4.2.1 实现机制

```cpp
bool IRGenerator::ir_and_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    ast_node * leftNode = node->sons[0];  // 左操作数
    ast_node * rightNode = node->sons[1]; // 右操作数
    
    Function * currentFunc = module->getCurrentFunction();
    
    // 创建中间标签L3
    LabelInstruction * rightLabel = new LabelInstruction(currentFunc, module);
    
    // 处理左操作数：如果为真跳转到rightLabel，为假跳转到falseLabel
    if (!ir_condition_expr(leftNode, rightLabel, falseLabel)) {
        return false;
    }
    node->blockInsts.addInst(leftNode->blockInsts);
    
    // 插入中间标签
    node->blockInsts.addInst(rightLabel);
    
    // 处理右操作数：如果为真跳转到trueLabel，为假跳转到falseLabel
    if (!ir_condition_expr(rightNode, trueLabel, falseLabel)) {
        return false;
    }
    node->blockInsts.addInst(rightNode->blockInsts);
    
    node->val = nullptr; // 条件表达式不设置val值
    
    return true;
}
```

#### 4.2.2 短路求值流程

**C代码：** `if (a > 0 && b > 0)`

**生成的IR逻辑：**
```llvm
; 计算 a > 0
%1 = icmp sgt i32 %a, 0
br i1 %1, label %check_b, label %false_branch

check_b:
; 计算 b > 0  
%2 = icmp sgt i32 %b, 0
br i1 %2, label %true_branch, label %false_branch

true_branch:
; 两个条件都为真的处理

false_branch:
; 至少一个条件为假的处理
```

**短路特性：** 如果`a > 0`为假，直接跳转到`false_branch`，不会计算`b > 0`。

### 4.3 逻辑或（||）的短路求值

#### 4.3.1 实现机制

```cpp
bool IRGenerator::ir_or_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    ast_node * leftNode = node->sons[0];  // 左操作数
    ast_node * rightNode = node->sons[1]; // 右操作数
    
    Function * currentFunc = module->getCurrentFunction();
    
    // 创建中间标签L3
    LabelInstruction * rightLabel = new LabelInstruction(currentFunc, module);
    
    // 处理左操作数：如果为真跳转到trueLabel，为假跳转到rightLabel
    if (!ir_condition_expr(leftNode, trueLabel, rightLabel)) {
        return false;
    }
    node->blockInsts.addInst(leftNode->blockInsts);
    
    // 插入中间标签
    node->blockInsts.addInst(rightLabel);
    
    // 处理右操作数：如果为真跳转到trueLabel，为假跳转到falseLabel
    if (!ir_condition_expr(rightNode, trueLabel, falseLabel)) {
        return false;
    }
    node->blockInsts.addInst(rightNode->blockInsts);
    
    node->val = nullptr; // 条件表达式不设置val值
    
    return true;
}
```

#### 4.3.2 短路求值流程

**C代码：** `if (a > 0 || b > 0)`

**生成的IR逻辑：**
```llvm
; 计算 a > 0
%1 = icmp sgt i32 %a, 0
br i1 %1, label %true_branch, label %check_b

check_b:
; 计算 b > 0
%2 = icmp sgt i32 %b, 0  
br i1 %2, label %true_branch, label %false_branch

true_branch:
; 至少一个条件为真的处理

false_branch:
; 两个条件都为假的处理
```

**短路特性：** 如果`a > 0`为真，直接跳转到`true_branch`，不会计算`b > 0`。

### 4.4 逻辑非（!）的实现

```cpp
bool IRGenerator::ir_not_with_labels(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    ast_node * operandNode = node->sons[0];
    
    // 注意：这里交换了trueLabel和falseLabel
    if (!ir_condition_expr(operandNode, falseLabel, trueLabel)) {
        return false;
    }
    node->blockInsts.addInst(operandNode->blockInsts);
    
    node->val = nullptr;
    
    return true;
}
```

**关键特性：** 通过交换真假标签实现逻辑非的语义。

## 5. 条件表达式统一处理

### 5.1 ir_condition_expr函数

这是整个条件处理的核心函数，负责将各种类型的表达式转换为条件跳转：

```cpp
bool IRGenerator::ir_condition_expr(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
    switch (node->node_type) {
        case ast_operator_type::AST_OP_AND:
            return ir_and_with_labels(node, trueLabel, falseLabel);
            
        case ast_operator_type::AST_OP_OR:
            return ir_or_with_labels(node, trueLabel, falseLabel);
            
        case ast_operator_type::AST_OP_NOT:
            return ir_not_with_labels(node, trueLabel, falseLabel);
            
        // 关系运算符直接处理
        case ast_operator_type::AST_OP_GT:
        case ast_operator_type::AST_OP_GE:
        case ast_operator_type::AST_OP_LT:
        case ast_operator_type::AST_OP_LE:
        case ast_operator_type::AST_OP_EQ:
        case ast_operator_type::AST_OP_NE:
            return ir_relop_with_labels(node, trueLabel, falseLabel);
            
        default:
            // 对于其他表达式，转换为条件跳转
            if (!ir_visit_ast_node(node)) {
                return false;
            }
            
            Value * condValue = nullptr;
            if (needsLoad(node->val)) {
                LoadInstruction * loadCond = new LoadInstruction(module->getCurrentFunction(), node->val, node->val, 4);
                node->blockInsts.addInst(loadCond);
                condValue = loadCond;
            } else {
                condValue = node->val;
            }
            
            // 转换为i1类型并生成条件跳转
            Value * boolValue = convertToI1(condValue, module->getCurrentFunction(), node->blockInsts);
            BranchInstruction * branchInst = new BranchInstruction(module->getCurrentFunction(), boolValue, trueLabel, falseLabel);
            node->blockInsts.addInst(branchInst);
            
            return true;
    }
}
```

### 5.2 类型转换处理

对于非布尔类型的表达式，需要转换为i1类型：

```cpp
Value * convertToI1(Value * value, Function * func, InterCode & blockInsts)
{
    if (value->getType()->isIntegerType()) {
        IntegerType * intType = static_cast<IntegerType *>(value->getType());
        if (intType->getBitWidth() == 1) {
            return value; // 已经是i1类型
        }
        
        // 生成比较指令：value != 0
        ConstInt * zeroConst = module->newConstInt(0);
        RelInstruction * cmpInst = new RelInstruction(func, IRInstOperator::IRINST_OP_NE, value, zeroConst);
        blockInsts.addInst(cmpInst);
        return cmpInst;
    }
    
    // 其他类型的处理...
}
```

## 6. 标签管理

### 6.1 全局标签计数器

为了避免标签名冲突，使用全局计数器生成唯一标签：

```cpp
LabelInstruction::LabelInstruction(Function * _func, Module * _module)
    : Instruction(_func, IRInstOperator::IRINST_OP_LABEL, VoidType::getType())
{
    // 使用全局标签计数器生成唯一标签名
    int32_t labelId = _module->getNextLabelId();
    setIRName(std::to_string(labelId));
}
```

### 6.2 循环标签栈

用于支持嵌套循环中的break和continue：

```cpp
struct LoopLabels {
    LabelInstruction * condLabel;  // 条件检查标签（continue目标）
    LabelInstruction * exitLabel;  // 退出标签（break目标）
};

std::stack<LoopLabels> loopLabelStack;
```

## 7. 指令类实现

### 7.1 BranchInstruction

```cpp
class BranchInstruction : public Instruction {
private:
    Value * condition;             // 条件表达式的值
    LabelInstruction * trueLabel;  // 条件为真时跳转的目标标签
    LabelInstruction * falseLabel; // 条件为假时跳转的目标标签

public:
    void toString(std::string & str) override {
        str = "br i1 " + condition->getIRName() + ", label %" + trueLabel->getIRName() + 
              ", label %" + falseLabel->getIRName();
    }
};
```

### 7.2 GotoInstruction

```cpp
class GotoInstruction : public Instruction {
private:
    LabelInstruction * target; // 跳转目标

public:
    void toString(std::string & str) override {
        str = "br label %" + target->getIRName();
    }
};
```

### 7.3 LabelInstruction

```cpp
class LabelInstruction : public Instruction {
public:
    void toString(std::string & str) override {
        str = getIRName() + ":";
    }
};
```

## 8. 复杂表达式示例

### 8.1 嵌套条件表达式

**C代码：**
```c
if ((a > 0 && b > 0) || (c < 0 && d < 0)) {
    x = 1;
}
```

**IR生成逻辑：**
1. 首先处理`(a > 0 && b > 0)`的短路求值
2. 如果第一部分为真，直接跳转到then分支
3. 如果第一部分为假，继续处理`(c < 0 && d < 0)`
4. 根据第二部分的结果决定最终跳转

### 8.2 循环中的复杂条件

**C代码：**
```c
while (i < n && arr[i] != target) {
    i++;
}
```

**短路求值保证：**
- 如果`i < n`为假，不会计算`arr[i] != target`，避免数组越界
- 生成的IR完全符合C语言的短路求值语义

## 9. 总结

本编译器的IR分支结构和短路求值实现具有以下特点：

1. **完整的短路求值**：完全符合C语言标准的短路求值语义
2. **高效的标签管理**：使用全局计数器避免标签冲突
3. **灵活的条件处理**：统一的条件表达式处理框架
4. **嵌套结构支持**：完整支持嵌套的循环和条件语句
5. **LLVM兼容**：生成的IR完全符合LLVM IR规范
6. **类型安全**：正确处理各种类型到布尔类型的转换

这个实现为编译器提供了强大而高效的控制流生成能力，是整个IR生成系统的重要组成部分。
