# 编译器前端文件详解

## 目录
1. [概述](#概述)
2. [前端文件结构](#前端文件结构)
3. [核心前端文件详解](#核心前端文件详解)
4. [ANTLR4相关文件](#antlr4相关文件)
5. [IR指令文件详解](#ir指令文件详解)
6. [文件依赖关系](#文件依赖关系)
7. [编译流程](#编译流程)

## 概述

本编译器采用模块化设计，前端负责词法分析、语法分析和AST生成，中间层负责IR生成和优化，后端负责目标代码生成。本文档详细介绍前端使用的所有文件以及IR指令系统的实现。

## 前端文件结构

```
frontend/
├── AST.h                    # AST节点定义和操作接口
├── AST.cpp                  # AST节点实现和工具函数
├── AttrType.h               # 属性类型定义
├── FrontEndExecutor.h       # 前端执行器接口
├── Graph.h                  # AST图形化显示接口
├── Graph.cpp                # AST图形化显示实现
└── antlr4/
    ├── MiniC.g4             # ANTLR4语法文件
    ├── Antlr4Executor.h     # ANTLR4执行器接口
    ├── Antlr4Executor.cpp   # ANTLR4执行器实现
    ├── Antlr4CSTVisitor.h   # CST访问器接口
    └── Antlr4CSTVisitor.cpp # CST访问器实现
```

## 核心前端文件详解

### 1. AST.h - 抽象语法树定义

**文件作用**：定义AST节点的数据结构和操作接口

**核心内容**：
- **AST节点类型枚举**：定义所有AST节点类型
- **ast_node类**：AST节点的核心数据结构
- **节点操作接口**：创建、删除、遍历AST节点的函数

**关键数据结构**：
```cpp
enum class ast_operator_type : int {
    // 叶子节点
    AST_OP_LEAF_LITERAL_UINT,    // 无符号整数字面量
    AST_OP_LEAF_LITERAL_FLOAT,   // 浮点数字面量
    AST_OP_LEAF_VAR_ID,          // 变量标识符
    AST_OP_LEAF_TYPE,            // 类型节点
    
    // 内部节点
    AST_OP_COMPILE_UNIT,         // 编译单元
    AST_OP_FUNC_DEF,             // 函数定义
    AST_OP_FUNC_CALL,            // 函数调用
    AST_OP_BLOCK,                // 语句块
    AST_OP_IF,                   // if语句
    AST_OP_WHILE,                // while循环
    AST_OP_FOR,                  // for循环
    AST_OP_ASSIGN,               // 赋值语句
    AST_OP_ADD,                  // 加法运算
    AST_OP_SUB,                  // 减法运算
    AST_OP_MUL,                  // 乘法运算
    AST_OP_DIV,                  // 除法运算
    // ... 更多运算符
};

class ast_node {
public:
    ast_operator_type node_type;     // 节点类型
    int64_t line_no;                 // 源代码行号
    Type * type;                     // 节点数据类型
    uint32_t integer_val;            // 整数字面量值
    float float_val;                 // 浮点数字面量值
    std::string name;                // 变量名或函数名
    ast_node * parent;               // 父节点指针
    std::vector<ast_node *> sons;    // 子节点列表
    InterCode blockInsts;            // IR指令块
    Value * val;                     // IR生成后的Value对象
    
    // 条件表达式标签（用于逻辑表达式）
    LabelInstruction * trueLabel;
    LabelInstruction * falseLabel;
};
```

**主要功能**：
- 提供AST节点的统一数据结构
- 支持各种类型的字面量和表达式
- 集成IR生成所需的信息
- 支持条件表达式的标签管理

### 2. AST.cpp - AST节点实现

**文件作用**：实现AST节点的创建、操作和管理功能

**核心功能**：
- **节点构造函数**：支持多种方式创建AST节点
- **节点管理**：插入子节点、删除节点、遍历节点
- **类型转换**：将属性类型转换为IR类型
- **工具函数**：AST相关的辅助功能

**关键函数**：
```cpp
// 节点创建函数
ast_node * ast_node::New(ast_operator_type type, ...);
ast_node * ast_node::New(std::string id, int64_t lineno);
ast_node * ast_node::New(Type * type);

// 节点管理函数
bool insert_son_node(ast_node * son);
void Delete(ast_node * node);

// 类型转换函数
Type * typeAttr2Type(type_attr & attr);
ast_node * create_type_node(type_attr & attr);

// 工具函数
ast_node * create_contain_node(ast_operator_type node_type, ...);
void free_ast(ast_node * root);
```

### 3. AttrType.h - 属性类型定义

**文件作用**：定义语法分析过程中使用的属性类型

**核心内容**：
- **基本属性类型**：整数、浮点数、标识符等属性
- **复合属性类型**：类型属性、数组属性等
- **属性转换接口**：属性到AST节点的转换

**关键数据结构**：
```cpp
// 整数字面量属性
struct digit_int_attr {
    uint32_t val;    // 整数值
    int64_t lineno;  // 行号
};

// 浮点数字面量属性
struct digit_float_attr {
    float val;       // 浮点数值
    int64_t lineno;  // 行号
};

// 类型属性
struct type_attr {
    Type * type;                    // 基本类型
    bool is_array;                  // 是否为数组
    std::vector<int> dimensions;    // 数组维度
};
```

### 4. FrontEndExecutor.h - 前端执行器接口

**文件作用**：定义前端执行器的抽象接口

**设计模式**：抽象工厂模式，支持多种前端实现

**核心接口**：
```cpp
class FrontEndExecutor {
public:
    FrontEndExecutor(std::string _filename);
    virtual ~FrontEndExecutor();
    
    // 核心执行函数
    virtual bool run() = 0;
    
    // 获取AST根节点
    ast_node * getASTRoot();
    
protected:
    std::string filename;    // 源文件路径
    ast_node * astRoot;      // AST根节点
};
```

### 5. Graph.h/Graph.cpp - AST图形化显示

**文件作用**：将AST转换为图形化表示，便于调试和可视化

**依赖库**：Graphviz图形库

**核心功能**：
- 遍历AST节点生成图形节点
- 创建节点间的连接关系
- 支持多种输出格式（PNG、SVG、PDF等）
- 提供节点属性的可视化显示

**主要函数**：
```cpp
// 图形化输出AST
void OutputAST(ast_node * root, std::string filePath);

// 内部实现函数
static Agnode_t * graph_visit_ast_node(Agraph_t * g, ast_node * node);
static std::string get_node_name(ast_node * node);
static std::string get_node_shape(ast_node * node);
```

## ANTLR4相关文件

### 1. MiniC.g4 - ANTLR4语法文件

**文件作用**：定义MiniC语言的词法和语法规则

**核心内容**：
- **词法规则**：Token定义，包括关键字、标识符、字面量等
- **语法规则**：语法产生式，定义语言的语法结构
- **优先级和结合性**：运算符的优先级和结合性规则

**关键规则示例**：
```antlr
// 词法规则
T_INT: 'int';
T_FLOAT: 'float';
T_IF: 'if';
T_WHILE: 'while';
T_ID: [a-zA-Z_][a-zA-Z0-9_]*;
T_DIGIT: [0-9]+;
T_FLOAT_DIGIT: [0-9]+ '.' [0-9]* ([eE] [+-]? [0-9]+)?;

// 语法规则
compileUnit: (decl)* EOF;
decl: varDecl | funcDef;
funcDef: type T_ID '(' formalParams? ')' block;
block: '{' blockItem* '}';
stmt: assignStmt | ifStmt | whileStmt | returnStmt | block;
expr: addExpr;
addExpr: mulExpr (('+' | '-') mulExpr)*;
```

### 2. Antlr4Executor.h/cpp - ANTLR4执行器

**文件作用**：实现基于ANTLR4的前端执行器

**继承关系**：继承自FrontEndExecutor接口

**核心流程**：
```cpp
bool Antlr4Executor::run() {
    // 1. 打开源文件
    std::ifstream ifs(filename);
    
    // 2. 创建ANTLR4输入流
    antlr4::ANTLRInputStream input{ifs};
    
    // 3. 创建词法分析器
    MiniCLexer lexer{&input};
    
    // 4. 创建Token流
    antlr4::CommonTokenStream tokenStream{&lexer};
    
    // 5. 创建语法分析器
    MiniCParser parser{&tokenStream};
    
    // 6. 进行语法分析，生成CST
    auto cstRoot = parser.compileUnit();
    
    // 7. 创建CST访问器
    MiniCCSTVisitor visitor;
    
    // 8. 遍历CST生成AST
    astRoot = visitor.run(cstRoot);
    
    return true;
}
```

### 3. Antlr4CSTVisitor.h/cpp - CST访问器

**文件作用**：将ANTLR4生成的CST转换为编译器内部的AST

**设计模式**：访问者模式

**核心功能**：
- 遍历CST的每个节点
- 将CST节点转换为对应的AST节点
- 处理语法分析过程中的属性传递
- 进行基本的语义检查

**关键访问函数**：
```cpp
class MiniCCSTVisitor : public MiniCBaseVisitor {
public:
    // 编译单元访问
    std::any visitCompileUnit(MiniCParser::CompileUnitContext * ctx) override;
    
    // 声明访问
    std::any visitDecl(MiniCParser::DeclContext * ctx) override;
    std::any visitVarDecl(MiniCParser::VarDeclContext * ctx) override;
    std::any visitFuncDef(MiniCParser::FuncDefContext * ctx) override;
    
    // 语句访问
    std::any visitBlock(MiniCParser::BlockContext * ctx) override;
    std::any visitStmt(MiniCParser::StmtContext * ctx) override;
    std::any visitAssignStmt(MiniCParser::AssignStmtContext * ctx) override;
    std::any visitIfStmt(MiniCParser::IfStmtContext * ctx) override;
    std::any visitWhileStmt(MiniCParser::WhileStmtContext * ctx) override;
    
    // 表达式访问
    std::any visitExpr(MiniCParser::ExprContext * ctx) override;
    std::any visitPrimaryExpression(MiniCParser::PrimaryExpressionContext * ctx) override;
    std::any visitFuncCall(MiniCParser::FuncCallContext * ctx) override;
    
    // 类型访问
    std::any visitType(MiniCParser::TypeContext * ctx) override;
};
```

**转换示例**：
```cpp
// 浮点数字面量的转换
std::any MiniCCSTVisitor::visitPrimaryExpression(MiniCParser::PrimaryExpressionContext * ctx) {
    if (ctx->T_FLOAT_DIGIT()) {
        // 获取浮点数文本
        std::string floatText = ctx->T_FLOAT_DIGIT()->getText();
        int64_t lineNo = ctx->T_FLOAT_DIGIT()->getSymbol()->getLine();
        
        // 解析浮点数值
        float val = std::stof(floatText);
        
        // 创建AST节点
        ast_node * node = ast_node::New(floatText, lineNo);
        node->float_val = val;
        node->node_type = ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT;
        
        return node;
    }
    // ... 处理其他类型的主表达式
}
```

## IR指令文件详解

IR指令系统位于`ir/Instructions/`目录下，实现了完整的LLVM IR指令集。

### 指令分类概览

| 指令类别 | 指令文件 | 功能描述 |
|----------|----------|----------|
| **内存管理** | AllocaInstruction | 栈内存分配 |
| | LoadInstruction | 内存加载 |
| | StoreInstruction | 内存存储 |
| **算术运算** | BinaryInstruction | 二元运算（加减乘除） |
| | XorInstruction | 异或运算 |
| **控制流** | BranchInstruction | 条件分支 |
| | GotoInstruction | 无条件跳转 |
| | LabelInstruction | 标签定义 |
| **函数调用** | FuncCallInstruction | 函数调用 |
| | ArgInstruction | 函数参数 |
| | ExitInstruction | 函数返回 |
| **类型转换** | SitofpInstruction | 整数转浮点数 |
| | FptosiInstruction | 浮点数转整数 |
| | ZextInstruction | 零扩展 |
| | SextInstruction | 符号扩展 |
| **指针操作** | GetelementptrInstruction | 指针运算 |
| | BitcastInstruction | 类型转换 |
| **内存操作** | MemcpyInstruction | 内存拷贝 |
| | MemsetInstruction | 内存设置 |
| **比较运算** | RelInstruction | 关系比较 |
| **数据移动** | MoveInstruction | 数据移动 |

### 核心指令详解

#### 1. AllocaInstruction - 内存分配指令

**功能**：在栈上分配内存空间

**LLVM IR格式**：`%result = alloca type, align alignment`

**实现细节**：
```cpp
class AllocaInstruction : public Instruction {
private:
    int align;  // 对齐值
    
public:
    AllocaInstruction(Function * _func, Value * _result, Type * _type, int _align = 4);
    void toString(std::string & str) override;
};

// 生成示例：%1 = alloca i32, align 4
```

**使用场景**：
- 局部变量声明
- 数组分配
- 临时变量存储

#### 2. BinaryInstruction - 二元运算指令

**功能**：实现各种二元算术和逻辑运算

**支持的运算**：
- 整数运算：`add nsw`, `sub nsw`, `mul nsw`, `sdiv`, `srem`
- 浮点运算：`fadd`, `fsub`, `fmul`, `fdiv`
- 逻辑运算：`xor`

**实现细节**：
```cpp
class BinaryInstruction : public Instruction {
public:
    BinaryInstruction(Function * _func, IRInstOperator _op, 
                     Value * _srcVal1, Value * _srcVal2, Type * _type);
    void toString(std::string & str) override;
};

// 生成示例：
// %3 = add nsw i32 %1, %2        (整数加法)
// %3 = fadd float %1, %2         (浮点加法)
```

#### 3. LoadInstruction/StoreInstruction - 内存访问指令

**LoadInstruction功能**：从内存加载数据到寄存器

**LLVM IR格式**：`%result = load type, type* %ptr, align alignment`

**StoreInstruction功能**：将数据存储到内存

**LLVM IR格式**：`store type %value, type* %ptr, align alignment`

**实现示例**：
```cpp
// Load指令生成：%2 = load i32, i32* %1, align 4
// Store指令生成：store i32 %1, i32* %2, align 4
```

#### 4. BranchInstruction - 条件分支指令

**功能**：根据条件进行分支跳转

**LLVM IR格式**：`br i1 %cond, label %true_label, label %false_label`

**实现细节**：
```cpp
class BranchInstruction : public Instruction {
private:
    Value * condition;              // 条件表达式
    LabelInstruction * trueLabel;   // 真分支标签
    LabelInstruction * falseLabel;  // 假分支标签
    
public:
    BranchInstruction(Function * _func, Value * _cond, 
                     LabelInstruction * _trueLabel, 
                     LabelInstruction * _falseLabel);
};

// 生成示例：br i1 %1, label %if.then, label %if.else
```

#### 5. FuncCallInstruction - 函数调用指令

**功能**：实现函数调用

**LLVM IR格式**：
- 有返回值：`%result = call return_type @func_name(param_list)`
- 无返回值：`call void @func_name(param_list)`

**实现细节**：
```cpp
class FuncCallInstruction : public Instruction {
private:
    Function * calledFunction;  // 被调用的函数
    
public:
    FuncCallInstruction(Function * _func, Function * _calledFunc, Type * _type);
    void toString(std::string & str) override;
};

// 生成示例：
// %1 = call i32 @add(i32 %0, i32 %1)
// call void @print(i32 %0)
```

#### 6. SitofpInstruction/FptosiInstruction - 类型转换指令

**SitofpInstruction功能**：有符号整数转浮点数

**LLVM IR格式**：`%result = sitofp i32 %src to float`

**FptosiInstruction功能**：浮点数转有符号整数

**LLVM IR格式**：`%result = fptosi float %src to i32`

**实现细节**：
```cpp
class SitofpInstruction : public Instruction {
private:
    Value * srcValue;   // 源值
    Type * targetType;  // 目标类型
    
public:
    SitofpInstruction(Function * _func, Value * _srcValue, Type * _targetType);
};

// 生成示例：
// %2 = sitofp i32 %1 to float
// %2 = fptosi float %1 to i32
```

#### 7. GetelementptrInstruction - 指针运算指令

**功能**：计算数组或结构体元素的地址

**LLVM IR格式**：`%result = getelementptr inbounds type, type* %ptr, indices...`

**使用场景**：
- 数组元素访问
- 多维数组索引
- 结构体成员访问

**实现示例**：
```cpp
// 一维数组：%3 = getelementptr inbounds [10 x i32], [10 x i32]* %1, i64 0, i64 %2
// 多维数组：%4 = getelementptr inbounds [5 x [10 x i32]], [5 x [10 x i32]]* %1, i64 0, i64 %2, i64 %3
```

#### 8. MemcpyInstruction/MemsetInstruction - 内存操作指令

**MemcpyInstruction功能**：内存拷贝

**LLVM IR格式**：`call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dest, i8* %src, i64 %size, i1 false)`

**MemsetInstruction功能**：内存设置

**LLVM IR格式**：`call void @llvm.memset.p0i8.i64(i8* %dest, i8 %value, i64 %size, i1 false)`

**使用场景**：
- 数组初始化
- 结构体拷贝
- 内存清零

### 指令操作码定义

```cpp
enum class IRInstOperator : std::int8_t {
    // 函数控制
    IRINST_OP_ENTRY,        // 函数入口
    IRINST_OP_EXIT,         // 函数出口
    IRINST_OP_LABEL,        // 标签
    IRINST_OP_GOTO,         // 无条件跳转
    IRINST_OP_BRANCH,       // 条件分支
    
    // 算术运算
    IRINST_OP_ADD_I,        // 整数加法
    IRINST_OP_SUB_I,        // 整数减法
    IRINST_OP_MUL_I,        // 整数乘法
    IRINST_OP_DIV_I,        // 整数除法
    IRINST_OP_MOD_I,        // 整数取模
    IRINST_OP_ADD_F,        // 浮点加法
    IRINST_OP_SUB_F,        // 浮点减法
    IRINST_OP_MUL_F,        // 浮点乘法
    IRINST_OP_DIV_F,        // 浮点除法
    
    // 逻辑运算
    IRINST_OP_XOR_I,        // 异或
    
    // 比较运算
    IRINST_OP_EQ,           // 等于
    IRINST_OP_NE,           // 不等于
    IRINST_OP_LT,           // 小于
    IRINST_OP_LE,           // 小于等于
    IRINST_OP_GT,           // 大于
    IRINST_OP_GE,           // 大于等于
    IRINST_OP_ICMP,         // 整数比较
    
    // 内存操作
    IRINST_OP_ALLOCA,       // 内存分配
    IRINST_OP_LOAD,         // 内存加载
    IRINST_OP_STORE,        // 内存存储
    
    // 函数调用
    IRINST_OP_FUNC_CALL,    // 函数调用
    IRINST_OP_ARG,          // 函数参数
    IRINST_OP_RET,          // 函数返回
    
    // 类型转换
    IRINST_OP_ZEXT,         // 零扩展
    IRINST_OP_SEXT,         // 符号扩展
    IRINST_OP_TRUNC,        // 截断
    IRINST_OP_SITOFP,       // 整数转浮点
    IRINST_OP_FPTOSI,       // 浮点转整数
    
    // 指针操作
    IRINST_OP_GEP,          // 指针运算
    IRINST_OP_BITCAST,      // 类型转换
    
    // 内存函数
    IRINST_OP_MEMCPY,       // 内存拷贝
    IRINST_OP_MEMSET,       // 内存设置
    
    // 数据移动
    IRINST_OP_ASSIGN,       // 赋值
};
```

## 文件依赖关系

### 前端文件依赖图

```
main.cpp
    ↓
FrontEndExecutor.h (接口)
    ↓
Antlr4Executor.h/cpp (实现)
    ↓
Antlr4CSTVisitor.h/cpp (CST→AST转换)
    ↓
AST.h/cpp (AST数据结构)
    ↓
AttrType.h (属性类型)
    ↓
Graph.h/cpp (可视化，可选)
```

### IR指令依赖关系

```
IRGenerator.cpp (IR生成器)
    ↓
Instruction.h (指令基类)
    ↓
具体指令类 (AllocaInstruction, BinaryInstruction, etc.)
    ↓
Value.h/Type.h (值和类型系统)
    ↓
Function.h/Module.h (函数和模块管理)
```

### 编译时依赖

**CMakeLists.txt中的依赖配置**：
```cmake
# 前端源代码集合
set(FRONTEND_SRCS
    # 前端共性代码
    frontend/AST.cpp
    frontend/AST.h
    frontend/Graph.cpp
    frontend/Graph.h
    frontend/FrontEndExecutor.h
    frontend/AttrType.h

    # ANTLR4相关代码
    ${ANTLR4_OUTPUT}
    frontend/antlr4/Antlr4CSTVisitor.cpp
    frontend/antlr4/Antlr4CSTVisitor.h
    frontend/antlr4/Antlr4Executor.cpp
    frontend/antlr4/Antlr4Executor.h
)

# IR指令源代码集合
set(IR_SRCS
    ir/Generator/IRGenerator.cpp
    ir/Instructions/AllocaInstruction.cpp
    ir/Instructions/BinaryInstruction.cpp
    ir/Instructions/BranchInstruction.cpp
    # ... 所有指令文件
)
```

## 编译流程

### 完整编译流程图

```
源代码文件 (.c)
    ↓ [词法分析]
Token流
    ↓ [语法分析]
具体语法树 (CST)
    ↓ [语义分析]
抽象语法树 (AST)
    ↓ [IR生成]
中间表示 (LLVM IR)
    ↓ [优化]
优化后的IR
    ↓ [代码生成]
目标代码 (ARM64汇编)
```

### 详细执行流程

#### 1. 主程序入口 (main.cpp)

```cpp
int main(int argc, char * argv[]) {
    // 1. 解析命令行参数
    parseCommandLine(argc, argv);

    // 2. 创建前端执行器
    FrontEndExecutor * frontEndExecutor = new Antlr4Executor(inputFile);

    // 3. 执行前端分析
    bool success = frontEndExecutor->run();
    if (!success) {
        minic_log(LOG_ERROR, "前端分析错误");
        return -1;
    }

    // 4. 获取AST根节点
    ast_node * astRoot = frontEndExecutor->getASTRoot();

    // 5. 可选：输出AST图形
    if (gShowAST) {
        OutputAST(astRoot, outputFile);
        return 0;
    }

    // 6. 创建IR生成器
    IRGenerator irGenerator;

    // 7. 生成IR
    bool irSuccess = irGenerator.run(astRoot);
    if (!irSuccess) {
        minic_log(LOG_ERROR, "IR生成错误");
        return -1;
    }

    // 8. 后端代码生成
    // ... 后端处理

    return 0;
}
```

#### 2. 前端执行流程 (Antlr4Executor::run)

```cpp
bool Antlr4Executor::run() {
    // Step 1: 文件输入
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        return false;
    }

    // Step 2: 创建ANTLR4输入流
    antlr4::ANTLRInputStream input{ifs};

    // Step 3: 词法分析
    MiniCLexer lexer{&input};
    antlr4::CommonTokenStream tokenStream{&lexer};

    // Step 4: 语法分析
    MiniCParser parser{&tokenStream};
    auto cstRoot = parser.compileUnit();

    // Step 5: CST到AST转换
    MiniCCSTVisitor visitor;
    astRoot = visitor.run(cstRoot);

    return astRoot != nullptr;
}
```

#### 3. CST到AST转换流程

```cpp
// 编译单元转换
std::any MiniCCSTVisitor::visitCompileUnit(MiniCParser::CompileUnitContext * ctx) {
    ast_node * compileUnitNode = create_contain_node(AST_OP_COMPILE_UNIT);

    // 遍历所有声明
    for (auto declCtx : ctx->decl()) {
        ast_node * declNode = std::any_cast<ast_node *>(visitDecl(declCtx));
        compileUnitNode->insert_son_node(declNode);
    }

    return compileUnitNode;
}

// 函数定义转换
std::any MiniCCSTVisitor::visitFuncDef(MiniCParser::FuncDefContext * ctx) {
    // 1. 获取返回类型
    ast_node * typeNode = std::any_cast<ast_node *>(visitType(ctx->type()));

    // 2. 获取函数名
    std::string funcName = ctx->T_ID()->getText();

    // 3. 处理参数列表
    ast_node * paramsNode = nullptr;
    if (ctx->formalParams()) {
        paramsNode = std::any_cast<ast_node *>(visitFormalParams(ctx->formalParams()));
    }

    // 4. 处理函数体
    ast_node * blockNode = std::any_cast<ast_node *>(visitBlock(ctx->block()));

    // 5. 创建函数定义节点
    ast_node * funcDefNode = create_contain_node(AST_OP_FUNC_DEF, typeNode, paramsNode, blockNode);
    funcDefNode->name = funcName;

    return funcDefNode;
}
```

#### 4. IR生成流程

```cpp
bool IRGenerator::run(ast_node * root) {
    // 1. 初始化模块和符号表
    module = new Module();

    // 2. 遍历AST生成IR
    bool success = ir_visit_ast_node(root);

    // 3. 输出IR到文件
    if (success) {
        module->outputIR(outputFile);
    }

    return success;
}

bool IRGenerator::ir_visit_ast_node(ast_node * node) {
    switch (node->node_type) {
        case AST_OP_COMPILE_UNIT:
            return ir_compile_unit(node);
        case AST_OP_FUNC_DEF:
            return ir_function_define(node);
        case AST_OP_BLOCK:
            return ir_block(node);
        case AST_OP_ASSIGN:
            return ir_assign(node);
        case AST_OP_ADD:
            return ir_add(node);
        // ... 处理所有节点类型
        default:
            return false;
    }
}
```

## 使用示例和测试

### 示例1：简单的C程序编译

**源代码 (test.c)**：
```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 10;
    int y = 20;
    int result = add(x, y);
    return result;
}
```

**编译命令**：
```bash
./minic -S test.c -o test.s
```

**AST生成过程**：
1. **词法分析**：识别Token序列
   ```
   T_INT, T_ID(add), T_LPAREN, T_INT, T_ID(a), T_COMMA, T_INT, T_ID(b), T_RPAREN, T_LBRACE, ...
   ```

2. **语法分析**：生成CST
   ```
   compileUnit
   ├── funcDef
   │   ├── type (int)
   │   ├── T_ID (add)
   │   ├── formalParams
   │   │   ├── formalParam (int a)
   │   │   └── formalParam (int b)
   │   └── block
   │       └── returnStmt
   │           └── expr (a + b)
   └── funcDef (main)
       └── ...
   ```

3. **AST转换**：生成内部AST
   ```
   AST_OP_COMPILE_UNIT
   ├── AST_OP_FUNC_DEF (add)
   │   ├── AST_OP_LEAF_TYPE (int)
   │   ├── AST_OP_FUNC_FORMAL_PARAMS
   │   │   ├── AST_OP_FUNC_FORMAL_PARAM (a, int)
   │   │   └── AST_OP_FUNC_FORMAL_PARAM (b, int)
   │   └── AST_OP_BLOCK
   │       └── AST_OP_RETURN
   │           └── AST_OP_ADD
   │               ├── AST_OP_LEAF_VAR_ID (a)
   │               └── AST_OP_LEAF_VAR_ID (b)
   └── AST_OP_FUNC_DEF (main)
       └── ...
   ```

4. **IR生成**：生成LLVM IR
   ```llvm
   define i32 @add(i32 %a, i32 %b) {
   entry:
     %0 = add nsw i32 %a, %b
     ret i32 %0
   }

   define i32 @main() {
   entry:
     %x = alloca i32, align 4
     %y = alloca i32, align 4
     %result = alloca i32, align 4
     store i32 10, i32* %x, align 4
     store i32 20, i32* %y, align 4
     %0 = load i32, i32* %x, align 4
     %1 = load i32, i32* %y, align 4
     %2 = call i32 @add(i32 %0, i32 %1)
     store i32 %2, i32* %result, align 4
     %3 = load i32, i32* %result, align 4
     ret i32 %3
   }
   ```

### 示例2：浮点数运算

**源代码**：
```c
float calculate(float x, float y) {
    float sum = x + y;
    float product = x * y;
    return sum / product;
}
```

**关键IR指令生成**：
```llvm
define float @calculate(float %x, float %y) {
entry:
  %sum = alloca float, align 4
  %product = alloca float, align 4
  %0 = fadd float %x, %y          ; 浮点加法
  store float %0, float* %sum, align 4
  %1 = fmul float %x, %y          ; 浮点乘法
  store float %1, float* %product, align 4
  %2 = load float, float* %sum, align 4
  %3 = load float, float* %product, align 4
  %4 = fdiv float %2, %3          ; 浮点除法
  ret float %4
}
```

### 示例3：数组操作

**源代码**：
```c
int sum_array(int arr[], int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}
```

**关键IR指令生成**：
```llvm
define i32 @sum_array(i32* %arr, i32 %size) {
entry:
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %sum, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:
  %0 = load i32, i32* %i, align 4
  %1 = icmp slt i32 %0, %size
  br i1 %1, label %for.body, label %for.end

for.body:
  %2 = load i32, i32* %i, align 4
  %3 = getelementptr inbounds i32, i32* %arr, i64 %2  ; 数组索引
  %4 = load i32, i32* %3, align 4
  %5 = load i32, i32* %sum, align 4
  %6 = add nsw i32 %5, %4
  store i32 %6, i32* %sum, align 4
  %7 = load i32, i32* %i, align 4
  %8 = add nsw i32 %7, 1
  store i32 %8, i32* %i, align 4
  br label %for.cond

for.end:
  %9 = load i32, i32* %sum, align 4
  ret i32 %9
}
```

## 调试和诊断

### AST可视化

使用Graph.cpp生成AST的图形化表示：

```bash
./minic --ast test.c -o test.png
```

生成的图形显示AST的树形结构，便于调试语法分析问题。

### IR输出和验证

生成的LLVM IR可以使用LLVM工具链验证：

```bash
# 生成IR文件
./minic -emit-llvm test.c -o test.ll

# 使用LLVM验证IR正确性
llvm-as test.ll -o test.bc
lli test.bc
```

### 错误处理机制

编译器在各个阶段都有完善的错误处理：

1. **词法错误**：无效字符、未闭合的字符串等
2. **语法错误**：语法规则不匹配、缺少分号等
3. **语义错误**：类型不匹配、未声明变量等
4. **IR生成错误**：无效的IR指令、类型错误等

每个错误都会提供详细的位置信息和错误描述，便于定位和修复问题。

## 总结

编译器前端的文件组织采用了清晰的模块化设计：

1. **接口层**：FrontEndExecutor定义统一接口
2. **实现层**：Antlr4Executor提供具体实现
3. **数据层**：AST定义核心数据结构
4. **转换层**：CSTVisitor实现语法树转换
5. **工具层**：Graph提供可视化支持

IR指令系统完整实现了LLVM IR的核心指令集，支持：
- 完整的算术和逻辑运算
- 灵活的内存管理
- 强大的类型转换
- 高效的函数调用
- 复杂的控制流

这套设计确保了编译器的可扩展性、可维护性和正确性，为后续的优化和目标代码生成奠定了坚实的基础。
```
