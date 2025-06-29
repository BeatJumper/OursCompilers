# 编译器前端浮点数功能详解

## 目录
1. [概述](#概述)
2. [浮点数词法分析](#浮点数词法分析)
3. [浮点数语法分析](#浮点数语法分析)
4. [浮点数类型系统](#浮点数类型系统)
5. [浮点数常量管理](#浮点数常量管理)
6. [浮点数运算](#浮点数运算)
7. [浮点数类型转换](#浮点数类型转换)
8. [浮点数IR生成](#浮点数ir生成)
9. [浮点数后端代码生成](#浮点数后端代码生成)
10. [调用流程总结](#调用流程总结)

## 概述

本编译器的浮点数功能支持完整的IEEE 754标准32位浮点数处理，包括：
- 浮点数字面量解析（十进制、科学计数法）
- 浮点数类型系统
- 浮点数常量管理和优化
- 浮点数四则运算
- 浮点数与整数的隐式/显式类型转换
- 浮点数LLVM IR生成
- ARM64浮点数指令生成

## 浮点数词法分析

### 文件位置
- `frontend/antlr4/MiniC.g4` - ANTLR4语法文件

### 核心实现

#### 1. 浮点数Token定义
```antlr
T_FLOAT_DIGIT:                                          // 定义浮点数Token的词法规则
    [0-9]+ '.' [0-9]* ([eE] [+-]? [0-9]+)?              // 第一种格式：整数部分.小数部分(可选指数)
    | '.' [0-9]+ ([eE] [+-]? [0-9]+)?                   // 第二种格式：.小数部分(可选指数)
    | [0-9]+ [eE] [+-]? [0-9]+;                         // 第三种格式：整数部分+必须的指数
```

**支持的浮点数格式：**
- 标准小数：`3.14159`, `2.0`, `0.5`
- 省略整数部分：`.5`, `.123`
- 科学计数法：`1.23e10`, `1e-5`, `.5e2`
- 正负指数：`1.23e+10`, `1.23e-5`

#### 2. 词法优先级
浮点数Token的优先级低于整数Token，确保`123`被识别为整数而不是浮点数的开始。

## 浮点数语法分析

### 文件位置
- `frontend/antlr4/Antlr4CSTVisitor.cpp` - ANTLR4访问器实现
- `frontend/AST.cpp` - AST节点构造函数实现

### 核心实现

#### 1. 浮点数字面量解析
```cpp
// 位置：Antlr4CSTVisitor.cpp:484-496
else if (ctx->T_FLOAT_DIGIT()) {                                    // 检查当前上下文是否包含浮点数Token
    // 获取浮点数字面量文本
    std::string floatText = ctx->T_FLOAT_DIGIT()->getText();        // 从Token中提取浮点数的文本表示
    int64_t lineNo = (int64_t) ctx->T_FLOAT_DIGIT()->getSymbol()->getLine(); // 获取Token在源代码中的行号

    // 解析浮点数值
    float val = 0.0f;                                               // 初始化浮点数值为0.0
    try {                                                           // 开始异常处理块
        val = std::stof(floatText);                                 // 使用C++标准库将字符串转换为float类型
    } catch (const std::exception & e) {                           // 捕获转换过程中的异常
        printf("Error: Failed to parse float '%s' at line %ld: %s\n",  // 打印错误信息
               floatText.c_str(), lineNo, e.what());               // 包含原始文本、行号和异常信息
        return nullptr;                                             // 返回空指针表示解析失败
    }
```

#### 2. AST节点创建
```cpp
// 创建浮点数AST节点
node = ast_node::New(floatText, lineNo);                            // 调用AST节点工厂方法创建新节点，传入文本和行号
node->float_val = val;                                              // 将解析得到的浮点数值存储到节点的float_val字段
node->node_type = ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT;     // 设置节点类型为浮点数字面量叶子节点
```

#### 3. AST节点数据结构
```cpp
// frontend/AST.h 中的节点定义
class ast_node {                                                   // AST节点类定义
public:                                                            // 公有成员
    ast_operator_type node_type;                                   // 节点类型枚举，标识节点的语法类别
    float float_val;                                               // 浮点数值存储字段（仅浮点数字面量节点使用）
    int32_t integer_val;                                           // 整数值存储字段（仅整数字面量节点使用）
    Value * val;                                                   // IR生成阶段产生的Value对象指针
    Type * type;                                                   // 节点的数据类型指针（如FloatType、IntType等）
    int64_t line_no;                                               // 节点对应的源代码行号，用于错误报告
    std::vector<ast_node *> sons;                                  // 子节点指针向量，构成AST的树形结构
};                                                                 // 类定义结束
```

#### 4. 浮点数AST节点构造函数
```cpp
// frontend/AST.cpp:53-57
ast_node::ast_node(digit_float_attr attr)                          // 浮点数属性构造函数，接收浮点数属性结构体
    : ast_node(ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT,        // 调用基础构造函数，设置节点类型为浮点数字面量
               FloatType::getTypeFloat(),                           // 设置节点的数据类型为FloatType
               attr.lineno)                                         // 设置节点的行号信息
{                                                                   // 构造函数体开始
    float_val = attr.val;                                           // 将属性结构体中的浮点数值赋给节点的float_val字段
}                                                                   // 构造函数体结束
```

**调用流程：**
1. ANTLR4词法分析器识别`T_FLOAT_DIGIT`
2. `Antlr4CSTVisitor::visitPrimaryExpression()`被调用
3. 检测到浮点数Token，调用`std::stof()`解析
4. 创建`AST_OP_LEAF_LITERAL_FLOAT`类型的AST节点
5. 将解析后的`float`值存储在`node->float_val`中
6. 设置节点类型为`FloatType::getTypeFloat()`

## 浮点数类型系统

### 文件位置
- `ir/Types/FloatType.h` - 浮点数类型定义
- `ir/Types/FloatType.cpp` - 浮点数类型实现

### 核心实现

#### 1. FloatType类设计
```cpp
class FloatType final : public Type {                              // 浮点数类型类，继承自Type基类，final表示不可被继承
public:                                                            // 公有成员区域
    static FloatType * getTypeFloat();                             // 静态方法，单例模式获取全局唯一的浮点类型实例

    std::string toString() const override { return "float"; }      // 重写基类方法，返回类型的字符串表示"float"
    bool isFloatType() { return true; }                            // 重写基类方法，标识这是浮点数类型
    int32_t getSize() const override { return 4; }                 // 重写基类方法，返回浮点数大小（32位=4字节）

    bool isSameType(Type * other) const;                           // 类型比较方法声明，检查是否与另一个类型相同
    bool canConvertTo(Type * target) const;                        // 类型转换检查方法声明，检查是否可转换为目标类型

private:                                                           // 私有成员区域
    FloatType() : Type(Type::FloatTyID) {}                         // 私有构造函数，调用基类构造函数并传入浮点类型ID
    static FloatType * oneInstance;                                // 静态成员变量，存储全局唯一的FloatType实例指针
};                                                                 // 类定义结束
```

#### 2. 类型转换支持
```cpp
// 位置：FloatType.cpp:49-52
bool FloatType::canConvertTo(Type * target) const                  // 检查浮点数类型是否可以转换为目标类型
{                                                                  // 函数体开始
    return target->isFloatType() || target->isIntegerType();       // 返回true如果目标类型是浮点数或整数类型
}                                                                  // 函数体结束
```

**特性：**
- **单例模式**：全局只有一个FloatType实例，节省内存
- **类型安全**：提供类型检查和转换支持
- **LLVM兼容**：生成标准的LLVM IR类型标识`float`

## 浮点数常量管理

### 文件位置
- `ir/Values/ConstFloat.h` - 浮点数常量类定义
- `symboltable/Module.cpp` - 常量管理实现

### 核心实现

#### 1. ConstFloat类设计
```cpp
class ConstFloat final : public Constant {                         // 浮点数常量类，继承自Constant基类，final表示不可继承
public:                                                            // 公有成员区域
    explicit ConstFloat(float val) : Constant(FloatType::getTypeFloat()) { // 显式构造函数，调用基类构造函数并传入浮点类型
        name = formatFloat(val);                                   // 调用格式化函数将浮点数转换为LLVM IR格式字符串
        floatVal = val;                                            // 存储原始浮点数值
    }                                                              // 构造函数结束

    std::string getIRName() const override { return name; }       // 重写基类方法，返回LLVM IR中的名称表示
    float getVal() const { return floatVal; }                     // 获取存储的浮点数值

private:                                                           // 私有成员区域
    static std::string formatFloat(float val) {                   // 静态方法，将浮点数格式化为LLVM IR格式
        char buffer[32];                                           // 创建32字节的字符缓冲区
        snprintf(buffer, sizeof(buffer), "%.6e", val);            // 使用科学计数法格式化浮点数，保留6位有效数字
        return buffer;                                             // 返回格式化后的字符串
    }                                                              // 格式化方法结束

    float floatVal;                                                // 存储原始浮点数值的成员变量
    std::string name;                                              // 存储LLVM IR格式字符串的成员变量
};                                                                 // 类定义结束
```

#### 2. 常量池管理
```cpp
// 位置：Module.cpp:221-234
ConstFloat * Module::newConstFloat(float floatVal)                  // 创建或获取浮点数常量的方法
{                                                                  // 函数体开始
    // 查找是否已存在相同常量
    ConstFloat * val = findConstFloat(floatVal);                   // 在常量池中查找是否已存在相同值的浮点数常量
    if (!val) {                                                    // 如果没有找到相同的常量
        // 不存在则创建新常量
        val = new ConstFloat(floatVal);                            // 创建新的ConstFloat对象
        insertConstFloatDirectly(val);                             // 将新常量直接插入到常量表中
        constFloatVector.push_back(val);                           // 将新常量添加到常量向量中用于内存管理
    }                                                              // if语句结束
    return val;                                                    // 返回找到的或新创建的常量指针
}                                                                  // 函数体结束
```

**特性：**
- **常量池优化**：相同值的浮点数常量只创建一次
- **LLVM格式**：自动格式化为LLVM IR要求的科学计数法格式
- **内存管理**：统一管理所有浮点数常量的生命周期

## 浮点数运算

### 文件位置
- `ir/Generator/IRGenerator.cpp` - IR生成器实现

### 核心实现

#### 1. 常量折叠优化
```cpp
// 位置：IRGenerator.cpp:560-568
if (leftConstFloat || rightConstFloat) {                           // 检查左右操作数中是否有浮点数常量
    float leftVal = leftConstFloat ? leftConstFloat->getVal() :    // 如果左操作数是浮点数常量，获取其值
                   static_cast<float>(leftConstInt->getVal());     // 否则将整数常量转换为浮点数
    float rightVal = rightConstFloat ? rightConstFloat->getVal() : // 如果右操作数是浮点数常量，获取其值
                    static_cast<float>(rightConstInt->getVal());   // 否则将整数常量转换为浮点数
    float result = leftVal + rightVal;                             // 在编译时计算加法结果

    ConstFloat * resultConst = module->newConstFloat(result);      // 创建结果常量对象
    node->val = resultConst;                                       // 将结果常量赋给AST节点的值字段
    printf("Debug: Constant folding result: %f + %f = %f\n",      // 打印调试信息显示常量折叠结果
           leftVal, rightVal, result);                             // 输出左值、右值和结果
    return true;                                                   // 返回true表示成功进行了常量折叠
}                                                                  // if语句结束
```

#### 2. 运行时浮点数运算
```cpp
// 浮点数加法指令生成
if (leftType->isFloatType() || rightType->isFloatType()) {         // 检查左右操作数中是否有浮点数类型
    // 确保两个操作数都是浮点数类型
    Value * leftFloat = convertToFloat(leftValue, currentFunc, node->blockInsts);  // 将左操作数转换为浮点数类型
    Value * rightFloat = convertToFloat(rightValue, currentFunc, node->blockInsts); // 将右操作数转换为浮点数类型

    // 生成浮点数加法指令
    Instruction * addInst = new AddInstruction(currentFunc, leftFloat, rightFloat); // 创建浮点数加法指令对象
    node->blockInsts.addInst(addInst);                             // 将指令添加到当前基本块的指令列表中
    node->val = addInst;                                           // 将指令对象赋给AST节点的值字段
}                                                                  // if语句结束
```

**支持的运算：**
- **四则运算**：`+`, `-`, `*`, `/`
- **常量折叠**：编译时计算常量表达式
- **类型提升**：整数自动提升为浮点数参与运算
- **混合运算**：支持整数和浮点数混合运算

## 浮点数类型转换

### 文件位置
- `ir/Generator/IRGenerator.cpp` - 类型转换实现
- `ir/Instructions/SitofpInstruction.h` - 整数转浮点数指令
- `ir/Instructions/FptosiInstruction.h` - 浮点数转整数指令

### 核心实现

#### 1. 整数转浮点数
```cpp
// 位置：IRGenerator.cpp:1504-1519
Value * IRGenerator::convertToFloat(Value * val, Function * func, InterCode & blockInsts) // 将值转换为浮点数类型的方法
{                                                                  // 函数体开始
    if (val->getType()->isFloatType()) {                          // 检查输入值是否已经是浮点数类型
        return val;                                                // 已经是浮点数，直接返回原值，无需转换
    }                                                              // if语句结束

    if (val->getType()->isIntegerType()) {                        // 检查输入值是否是整数类型
        // 整数转浮点数 - 使用 sitofp 指令
        Instruction * convInst = new SitofpInstruction(func, val, FloatType::getTypeFloat()); // 创建sitofp指令对象
        blockInsts.addInst(convInst);                              // 将转换指令添加到指令块中
        return convInst;                                           // 返回转换指令对象作为结果值
    }                                                              // if语句结束

    printf("Error: Cannot convert value to float type\n");        // 打印错误信息，表示无法转换的类型
    return nullptr;                                                // 返回空指针表示转换失败
}                                                                  // 函数体结束
```

#### 2. 浮点数转整数
```cpp
// 位置：IRGenerator.cpp:1521-1536
Value * IRGenerator::convertToInt(Value * val, Function * func, InterCode & blockInsts) // 将值转换为整数类型的方法
{                                                                  // 函数体开始
    if (val->getType()->isIntegerType()) {                        // 检查输入值是否已经是整数类型
        return val;                                                // 已经是整数，直接返回原值，无需转换
    }                                                              // if语句结束

    if (val->getType()->isFloatType()) {                          // 检查输入值是否是浮点数类型
        // 浮点数转整数 - 使用 fptosi 指令
        Instruction * convInst = new FptosiInstruction(func, val, IntegerType::getTypeInt()); // 创建fptosi指令对象
        blockInsts.addInst(convInst);                              // 将转换指令添加到指令块中
        return convInst;                                           // 返回转换指令对象作为结果值
    }                                                              // if语句结束

    printf("Error: Cannot convert value to integer type\n");      // 打印错误信息，表示无法转换的类型
    return nullptr;                                                // 返回空指针表示转换失败
}                                                                  // 函数体结束
```

**转换类型：**
- **sitofp**：有符号整数转浮点数（Signed Integer TO Float Point）
- **fptosi**：浮点数转有符号整数（Float Point TO Signed Integer）
- **隐式转换**：运算时自动进行类型提升
- **显式转换**：强制类型转换

## 浮点数IR生成

### 文件位置
- `ir/Generator/IRGenerator.cpp` - IR生成主逻辑

### 核心实现

#### 1. 浮点数字面量IR生成
```cpp
// 位置：IRGenerator.cpp:1679-1693
bool IRGenerator::ir_leaf_node_float(ast_node * node)             // 处理浮点数字面量AST节点的IR生成方法
{                                                                  // 函数体开始
    float value = node->float_val;                                 // 从AST节点中获取浮点数值

    // 新建浮点数常量
    ConstFloat * newConst = module->newConstFloat(value);          // 调用模块的方法创建浮点数常量对象

    // 设置节点的值
    node->val = newConst;                                          // 将创建的常量对象赋给AST节点的值字段

    return true;                                                   // 返回true表示IR生成成功
}                                                                  // 函数体结束
```

#### 2. 浮点数运算IR生成
生成的LLVM IR示例：
```llvm
; 浮点数常量
%1 = fadd float 3.141590e+00, 2.718280e+00

; 类型转换
%2 = sitofp i32 %0 to float
%3 = fptosi float %1 to i32

; 浮点数运算
%4 = fadd float %2, %3
%5 = fsub float %2, %3
%6 = fmul float %2, %3
%7 = fdiv float %2, %3
```

**调用流程：**
1. AST遍历到浮点数字面量节点
2. 调用`ir_leaf_node_float()`
3. 从`node->float_val`获取浮点数值
4. 调用`module->newConstFloat()`创建常量
5. 将常量赋值给`node->val`

## 详细示例分析

### 示例1：浮点数常量声明和初始化

#### 源代码
```c
const float PI = 3.14159;
float radius = 2.5;
float area = PI * radius * radius;
```

#### 详细处理流程

**阶段1：词法分析（MiniC.g4）**
```
源代码: "const float PI = 3.14159;"
    ↓ [ANTLR4词法分析器]
Token序列: [T_CONST, T_FLOAT, T_ID(PI), T_ASSIGN, T_FLOAT_DIGIT(3.14159), T_SEMICOLON]
```

**阶段2：语法分析（Antlr4CSTVisitor.cpp）**

*文件：frontend/antlr4/Antlr4CSTVisitor.cpp*
*函数：visitVarDecl()*
```cpp
std::any MiniCCSTVisitor::visitVarDecl(MiniCParser::VarDeclContext * ctx) {
    // 处理const关键字
    bool isConst = ctx->T_CONST() != nullptr;                     // 检查是否有const修饰符

    // 获取类型信息
    ast_node * typeNode = std::any_cast<ast_node *>(visitType(ctx->type())); // 访问类型节点，获取float类型

    // 获取变量名
    std::string varName = ctx->T_ID()->getText();                 // 从Token中提取变量名"PI"

    // 处理初始化表达式
    ast_node * initNode = nullptr;                                // 初始化表达式节点指针
    if (ctx->expr()) {                                            // 检查是否有初始化表达式
        initNode = std::any_cast<ast_node *>(visitExpr(ctx->expr())); // 访问初始化表达式"3.14159"
    }

    // 创建变量声明节点
    ast_node * varDeclNode = create_contain_node(AST_OP_VAR_DECL, typeNode, initNode); // 创建变量声明AST节点
    varDeclNode->name = varName;                                  // 设置变量名
    varDeclNode->isConst = isConst;                               // 设置常量标志

    return varDeclNode;                                           // 返回创建的变量声明节点
}
```

*函数：visitPrimaryExp() - 处理浮点数字面量*
```cpp
std::any MiniCCSTVisitor::visitPrimaryExp(MiniCParser::PrimaryExpContext * ctx) {
    if (ctx->T_FLOAT_DIGIT()) {                                   // 检查是否是浮点数字面量
        std::string floatText = ctx->T_FLOAT_DIGIT()->getText();  // 获取"3.14159"文本
        int64_t lineNo = ctx->T_FLOAT_DIGIT()->getSymbol()->getLine(); // 获取行号

        float val = 0.0f;                                         // 初始化浮点数值
        try {                                                     // 异常处理开始
            val = std::stof(floatText);                           // 将"3.14159"转换为float类型
        } catch (const std::exception & e) {                     // 捕获转换异常
            printf("Error: Failed to parse float '%s' at line %ld: %s\n",
                   floatText.c_str(), lineNo, e.what());         // 打印错误信息
            return nullptr;                                       // 返回空指针
        }

        // 创建浮点数字面量节点
        ast_node * node = ast_node::New(floatText, lineNo);       // 创建AST节点
        node->float_val = val;                                    // 存储浮点数值3.14159
        node->node_type = AST_OP_LEAF_LITERAL_FLOAT;              // 设置节点类型
        node->type = FloatType::getTypeFloat();                   // 设置数据类型

        return node;                                              // 返回浮点数字面量节点
    }
}
```

**阶段3：AST构建（AST.cpp）**

*文件：frontend/AST.cpp*
*函数：ast_node构造函数*
```cpp
ast_node::ast_node(digit_float_attr attr)                        // 浮点数属性构造函数
    : ast_node(AST_OP_LEAF_LITERAL_FLOAT,                         // 设置节点类型为浮点数字面量
               FloatType::getTypeFloat(),                         // 设置类型为FloatType
               attr.lineno)                                       // 设置行号
{                                                                 // 构造函数体开始
    float_val = attr.val;                                         // 存储浮点数值
}                                                                 // 构造函数体结束
```

**阶段4：IR生成（IRGenerator.cpp）**

*文件：ir/Generator/IRGenerator.cpp*
*函数：ir_var_decl() - 处理变量声明*
```cpp
bool IRGenerator::ir_var_decl(ast_node * node) {
    std::string varName = node->name;                             // 获取变量名"PI"
    Type * varType = node->type;                                  // 获取变量类型FloatType
    bool isConst = node->isConst;                                 // 获取常量标志

    if (varType->isFloatType()) {                                 // 检查是否是浮点数类型
        // 创建局部变量对象
        LocalVariable * localVar = new LocalVariable(varName, varType); // 创建局部变量对象
        localVar->setConst(isConst);                              // 设置常量属性

        // 生成alloca指令分配内存
        AllocaInstruction * allocaInst = new AllocaInstruction(   // 创建内存分配指令
            currentFunc,                                          // 当前函数
            localVar,                                             // 变量对象
            varType,                                              // 变量类型
            4);                                                   // 4字节对齐
        node->blockInsts.addInst(allocaInst);                     // 添加到指令块

        // 处理初始化值
        if (node->sons.size() > 0) {                              // 检查是否有初始化表达式
            ast_node * initNode = node->sons[0];                  // 获取初始化节点
            ir_visit_ast_node(initNode);                          // 递归处理初始化表达式

            Value * initValue = initNode->val;                    // 获取初始化值

            // 生成store指令存储初始值
            StoreInstruction * storeInst = new StoreInstruction(  // 创建存储指令
                currentFunc,                                      // 当前函数
                initValue,                                        // 要存储的值
                localVar,                                         // 目标变量
                4);                                               // 4字节对齐
            node->blockInsts.addInst(storeInst);                  // 添加到指令块
        }

        // 注册变量到符号表
        module->addLocalVariable(localVar);                       // 将变量添加到模块的符号表
        node->val = localVar;                                     // 设置节点的值为变量对象
    }

    return true;                                                  // 返回成功
}
```

*函数：ir_leaf_node_float() - 处理浮点数字面量*
```cpp
bool IRGenerator::ir_leaf_node_float(ast_node * node) {
    float value = node->float_val;                                // 从AST节点获取浮点数值3.14159

    // 创建浮点数常量
    ConstFloat * newConst = module->newConstFloat(value);         // 调用模块方法创建常量对象

    // 设置节点的值
    node->val = newConst;                                         // 将常量对象赋给节点的val字段

    return true;                                                  // 返回成功
}
```

**阶段5：常量管理（Module.cpp）**

*文件：symboltable/Module.cpp*
*函数：newConstFloat() - 创建浮点数常量*
```cpp
ConstFloat * Module::newConstFloat(float floatVal) {
    // 查找是否已存在相同常量
    ConstFloat * val = findConstFloat(floatVal);                  // 在常量池中查找相同值的常量
    if (!val) {                                                   // 如果没有找到
        // 创建新常量
        val = new ConstFloat(floatVal);                           // 创建新的ConstFloat对象
        insertConstFloatDirectly(val);                            // 插入到常量表
        constFloatVector.push_back(val);                          // 添加到常量向量
    }
    return val;                                                   // 返回常量对象
}
```

*函数：ConstFloat构造函数*
```cpp
ConstFloat::ConstFloat(float val) : Constant(FloatType::getTypeFloat()) { // 调用基类构造函数
    name = formatFloat(val);                                      // 格式化浮点数为LLVM IR格式
    floatVal = val;                                               // 存储原始浮点数值
}

std::string ConstFloat::formatFloat(float val) {
    char buffer[32];                                              // 创建字符缓冲区
    snprintf(buffer, sizeof(buffer), "%.6e", val);               // 格式化为科学计数法
    return buffer;                                                // 返回格式化字符串"3.141590e+00"
}
```

**生成的LLVM IR：**
```llvm
; 常量声明 const float PI = 3.14159;
%PI = alloca float, align 4                    ; 分配4字节内存给PI变量
store float 3.141590e+00, float* %PI, align 4 ; 存储常量值到PI变量

; 变量声明 float radius = 2.5;
%radius = alloca float, align 4                ; 分配4字节内存给radius变量
store float 2.500000e+00, float* %radius, align 4 ; 存储常量值到radius变量
```

### 示例2：浮点数运算和常量折叠

#### 源代码
```c
float x = 1.5;
float y = 2.5;
float sum = x + y;
float product = 3.14 * 2.0;  // 常量折叠
```

#### 详细处理流程

**阶段1：运算表达式解析（Antlr4CSTVisitor.cpp）**

*文件：frontend/antlr4/Antlr4CSTVisitor.cpp*
*函数：visitAddExpr() - 处理加法表达式*
```cpp
std::any MiniCCSTVisitor::visitAddExpr(MiniCParser::AddExprContext * ctx) {
    if (ctx->addExpr() && ctx->mulExpr()) {                       // 检查是否是二元加法表达式
        // 获取左右操作数
        ast_node * leftNode = std::any_cast<ast_node *>(visitAddExpr(ctx->addExpr())); // 递归访问左操作数
        ast_node * rightNode = std::any_cast<ast_node *>(visitMulExpr(ctx->mulExpr())); // 访问右操作数

        // 创建加法运算节点
        ast_node * addNode = create_contain_node(AST_OP_ADD, leftNode, rightNode); // 创建加法AST节点

        // 设置运算符信息
        if (ctx->T_ADD()) {                                       // 检查是否是加号
            addNode->node_type = AST_OP_ADD;                      // 设置节点类型为加法
        }

        return addNode;                                           // 返回加法节点
    }

    // 如果不是二元表达式，继续向下递归
    return visitMulExpr(ctx->mulExpr());                          // 访问乘法表达式
}
```

**阶段2：IR生成 - 运算处理（IRGenerator.cpp）**

*文件：ir/Generator/IRGenerator.cpp*
*函数：ir_add() - 处理加法运算*
```cpp
bool IRGenerator::ir_add(ast_node * node) {
    // 处理左右操作数
    ast_node * leftNode = node->sons[0];                         // 获取左操作数节点
    ast_node * rightNode = node->sons[1];                        // 获取右操作数节点

    // 递归生成左右操作数的IR
    ir_visit_ast_node(leftNode);                                 // 生成左操作数的IR
    ir_visit_ast_node(rightNode);                                // 生成右操作数的IR

    Value * leftValue = leftNode->val;                           // 获取左操作数的值
    Value * rightValue = rightNode->val;                         // 获取右操作数的值

    // 尝试常量折叠优化
    ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(leftValue);  // 检查左操作数是否是浮点数常量
    ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(rightValue); // 检查右操作数是否是浮点数常量
    ConstInt * leftConstInt = dynamic_cast<ConstInt *>(leftValue);        // 检查左操作数是否是整数常量
    ConstInt * rightConstInt = dynamic_cast<ConstInt *>(rightValue);      // 检查右操作数是否是整数常量

    // 常量折叠：如果两个操作数都是常量，在编译时计算结果
    if ((leftConstFloat || leftConstInt) && (rightConstFloat || rightConstInt)) {
        float leftVal = leftConstFloat ? leftConstFloat->getVal() :       // 获取左操作数的浮点数值
                       static_cast<float>(leftConstInt->getVal());        // 或将整数转换为浮点数
        float rightVal = rightConstFloat ? rightConstFloat->getVal() :    // 获取右操作数的浮点数值
                        static_cast<float>(rightConstInt->getVal());      // 或将整数转换为浮点数
        float result = leftVal + rightVal;                        // 编译时计算加法结果

        ConstFloat * resultConst = module->newConstFloat(result); // 创建结果常量
        node->val = resultConst;                                  // 将结果赋给节点

        printf("Debug: Constant folding: %f + %f = %f\n",        // 打印调试信息
               leftVal, rightVal, result);
        return true;                                              // 返回成功
    }

    // 运行时计算：生成加法指令
    Type * leftType = leftValue->getType();                      // 获取左操作数类型
    Type * rightType = rightValue->getType();                    // 获取右操作数类型

    if (leftType->isFloatType() || rightType->isFloatType()) {   // 检查是否有浮点数参与
        // 类型提升：确保两个操作数都是浮点数
        Value * leftFloat = convertToFloat(leftValue, currentFunc, node->blockInsts);  // 转换左操作数为浮点数
        Value * rightFloat = convertToFloat(rightValue, currentFunc, node->blockInsts); // 转换右操作数为浮点数

        // 生成浮点数加法指令
        BinaryInstruction * addInst = new BinaryInstruction(     // 创建二元运算指令
            currentFunc,                                          // 当前函数
            IRInstOperator::IRINST_OP_ADD_F,                     // 浮点数加法操作码
            leftFloat,                                            // 左操作数
            rightFloat,                                           // 右操作数
            FloatType::getTypeFloat());                           // 结果类型为浮点数

        node->blockInsts.addInst(addInst);                        // 添加指令到基本块
        node->val = addInst;                                      // 设置节点值为指令对象
    } else {
        // 整数加法
        BinaryInstruction * addInst = new BinaryInstruction(     // 创建整数加法指令
            currentFunc,                                          // 当前函数
            IRInstOperator::IRINST_OP_ADD_I,                     // 整数加法操作码
            leftValue,                                            // 左操作数
            rightValue,                                           // 右操作数
            IntegerType::getTypeInt());                           // 结果类型为整数

        node->blockInsts.addInst(addInst);                        // 添加指令到基本块
        node->val = addInst;                                      // 设置节点值为指令对象
    }

    return true;                                                  // 返回成功
}
```

**生成的LLVM IR：**
```llvm
; 变量声明
%x = alloca float, align 4
%y = alloca float, align 4
%sum = alloca float, align 4
%product = alloca float, align 4

; 初始化
store float 1.500000e+00, float* %x, align 4
store float 2.500000e+00, float* %y, align 4

; 运行时加法：sum = x + y
%1 = load float, float* %x, align 4
%2 = load float, float* %y, align 4
%3 = fadd float %1, %2
store float %3, float* %sum, align 4

; 常量折叠：product = 3.14 * 2.0 = 6.28（编译时计算）
store float 6.280000e+00, float* %product, align 4
```

### 示例3：浮点数与整数混合运算

#### 源代码
```c
int count = 10;
float rate = 1.5;
float result = count * rate;
int rounded = (int)(result + 0.5);
```

#### 详细处理流程

**阶段1：混合运算表达式解析（Antlr4CSTVisitor.cpp）**

*文件：frontend/antlr4/Antlr4CSTVisitor.cpp*
*函数：visitMulExpr() - 处理乘法表达式*
```cpp
std::any MiniCCSTVisitor::visitMulExpr(MiniCParser::MulExprContext * ctx) {
    if (ctx->mulExpr() && ctx->unaryExpr()) {                     // 检查是否是二元乘法表达式
        // 获取左右操作数
        ast_node * leftNode = std::any_cast<ast_node *>(visitMulExpr(ctx->mulExpr())); // 递归访问左操作数
        ast_node * rightNode = std::any_cast<ast_node *>(visitUnaryExpr(ctx->unaryExpr())); // 访问右操作数

        // 创建乘法运算节点
        ast_node * mulNode = create_contain_node(AST_OP_MUL, leftNode, rightNode); // 创建乘法AST节点

        // 设置运算符信息
        if (ctx->T_MUL()) {                                       // 检查是否是乘号
            mulNode->node_type = AST_OP_MUL;                      // 设置节点类型为乘法
        }

        return mulNode;                                           // 返回乘法节点
    }

    return visitUnaryExpr(ctx->unaryExpr());                      // 继续向下递归
}
```

**阶段2：类型转换处理（IRGenerator.cpp）**

*文件：ir/Generator/IRGenerator.cpp*
*函数：ir_mul() - 处理乘法运算*
```cpp
bool IRGenerator::ir_mul(ast_node * node) {
    // 处理左右操作数
    ast_node * leftNode = node->sons[0];                         // 获取左操作数节点（count）
    ast_node * rightNode = node->sons[1];                        // 获取右操作数节点（rate）

    // 递归生成左右操作数的IR
    ir_visit_ast_node(leftNode);                                 // 生成左操作数的IR
    ir_visit_ast_node(rightNode);                                // 生成右操作数的IR

    Value * leftValue = leftNode->val;                           // 获取左操作数的值（整数类型）
    Value * rightValue = rightNode->val;                         // 获取右操作数的值（浮点数类型）

    Type * leftType = leftValue->getType();                      // 获取左操作数类型（IntegerType）
    Type * rightType = rightValue->getType();                    // 获取右操作数类型（FloatType）

    // 检查是否需要类型转换
    if (leftType->isFloatType() || rightType->isFloatType()) {   // 有浮点数参与运算
        // 类型提升：将整数转换为浮点数
        Value * leftFloat = convertToFloat(leftValue, currentFunc, node->blockInsts);  // 转换count为浮点数
        Value * rightFloat = convertToFloat(rightValue, currentFunc, node->blockInsts); // rate已经是浮点数

        // 生成浮点数乘法指令
        BinaryInstruction * mulInst = new BinaryInstruction(     // 创建浮点数乘法指令
            currentFunc,                                          // 当前函数
            IRInstOperator::IRINST_OP_MUL_F,                     // 浮点数乘法操作码
            leftFloat,                                            // 转换后的左操作数
            rightFloat,                                           // 右操作数
            FloatType::getTypeFloat());                           // 结果类型为浮点数

        node->blockInsts.addInst(mulInst);                        // 添加指令到基本块
        node->val = mulInst;                                      // 设置节点值为指令对象
    }

    return true;                                                  // 返回成功
}
```

*函数：convertToFloat() - 整数转浮点数*
```cpp
Value * IRGenerator::convertToFloat(Value * val, Function * func, InterCode & blockInsts) {
    if (val->getType()->isFloatType()) {                         // 检查是否已经是浮点数类型
        return val;                                               // 已经是浮点数，直接返回
    }

    if (val->getType()->isIntegerType()) {                       // 检查是否是整数类型
        // 创建sitofp指令进行类型转换
        SitofpInstruction * convInst = new SitofpInstruction(    // 创建整数转浮点数指令
            func,                                                 // 当前函数
            val,                                                  // 源值（整数）
            FloatType::getTypeFloat());                           // 目标类型（浮点数）

        blockInsts.addInst(convInst);                             // 添加转换指令到基本块
        return convInst;                                          // 返回转换指令对象
    }

    printf("Error: Cannot convert value to float type\n");       // 打印错误信息
    return nullptr;                                               // 返回空指针表示失败
}
```

**阶段3：强制类型转换处理**

*函数：visitCastExpr() - 处理强制类型转换*
```cpp
std::any MiniCCSTVisitor::visitCastExpr(MiniCParser::CastExprContext * ctx) {
    if (ctx->type() && ctx->expr()) {                            // 检查是否是类型转换表达式
        // 获取目标类型
        ast_node * targetTypeNode = std::any_cast<ast_node *>(visitType(ctx->type())); // 访问目标类型（int）

        // 获取源表达式
        ast_node * sourceExpr = std::any_cast<ast_node *>(visitExpr(ctx->expr())); // 访问源表达式

        // 创建类型转换节点
        ast_node * castNode = create_contain_node(AST_OP_CAST, targetTypeNode, sourceExpr); // 创建类型转换节点
        castNode->type = targetTypeNode->type;                    // 设置转换后的类型

        return castNode;                                          // 返回类型转换节点
    }

    return visitUnaryExpr(ctx->unaryExpr());                      // 继续向下递归
}
```

*函数：ir_cast() - 处理类型转换IR生成*
```cpp
bool IRGenerator::ir_cast(ast_node * node) {
    ast_node * targetTypeNode = node->sons[0];                   // 获取目标类型节点
    ast_node * sourceExprNode = node->sons[1];                   // 获取源表达式节点

    // 生成源表达式的IR
    ir_visit_ast_node(sourceExprNode);                           // 递归生成源表达式IR

    Value * sourceValue = sourceExprNode->val;                   // 获取源值
    Type * sourceType = sourceValue->getType();                  // 获取源类型
    Type * targetType = targetTypeNode->type;                    // 获取目标类型

    // 执行类型转换
    if (sourceType->isFloatType() && targetType->isIntegerType()) { // 浮点数转整数
        FptosiInstruction * convInst = new FptosiInstruction(    // 创建浮点数转整数指令
            currentFunc,                                          // 当前函数
            sourceValue,                                          // 源值（浮点数）
            targetType);                                          // 目标类型（整数）

        node->blockInsts.addInst(convInst);                       // 添加转换指令到基本块
        node->val = convInst;                                     // 设置节点值为转换指令
    } else if (sourceType->isIntegerType() && targetType->isFloatType()) { // 整数转浮点数
        SitofpInstruction * convInst = new SitofpInstruction(    // 创建整数转浮点数指令
            currentFunc,                                          // 当前函数
            sourceValue,                                          // 源值（整数）
            targetType);                                          // 目标类型（浮点数）

        node->blockInsts.addInst(convInst);                       // 添加转换指令到基本块
        node->val = convInst;                                     // 设置节点值为转换指令
    } else {
        // 相同类型或其他情况，直接使用源值
        node->val = sourceValue;                                  // 直接使用源值
    }

    return true;                                                  // 返回成功
}
```

**生成的LLVM IR：**
```llvm
; 变量声明
%count = alloca i32, align 4
%rate = alloca float, align 4
%result = alloca float, align 4
%rounded = alloca i32, align 4

; 初始化
store i32 10, i32* %count, align 4
store float 1.500000e+00, float* %rate, align 4

; 混合运算：result = count * rate
%1 = load i32, i32* %count, align 4          ; 加载整数count
%2 = sitofp i32 %1 to float                  ; 整数转浮点数
%3 = load float, float* %rate, align 4       ; 加载浮点数rate
%4 = fmul float %2, %3                       ; 浮点数乘法
store float %4, float* %result, align 4      ; 存储结果

; 强制类型转换：rounded = (int)(result + 0.5)
%5 = load float, float* %result, align 4     ; 加载result
%6 = fadd float %5, 5.000000e-01             ; 加上0.5
%7 = fptosi float %6 to i32                  ; 浮点数转整数
store i32 %7, i32* %rounded, align 4         ; 存储转换结果
```

### 示例4：浮点数数组操作

#### 源代码
```c
float numbers[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
float sum = 0.0;
for (int i = 0; i < 5; i++) {
    sum += numbers[i];
}
```

#### 详细处理流程

**阶段1：数组声明解析（Antlr4CSTVisitor.cpp）**

*文件：frontend/antlr4/Antlr4CSTVisitor.cpp*
*函数：visitVarDecl() - 处理数组声明*
```cpp
std::any MiniCCSTVisitor::visitVarDecl(MiniCParser::VarDeclContext * ctx) {
    // 获取基本类型
    ast_node * typeNode = std::any_cast<ast_node *>(visitType(ctx->type())); // 获取float类型
    Type * baseType = typeNode->type;                             // 获取基本类型FloatType

    // 获取变量名
    std::string varName = ctx->T_ID()->getText();                 // 获取变量名"numbers"

    // 检查是否是数组声明
    if (ctx->arrayDimensions()) {                                 // 检查是否有数组维度
        std::vector<int> dimensions;                              // 创建维度向量

        // 处理数组维度
        for (auto dimCtx : ctx->arrayDimensions()->expr()) {      // 遍历每个维度表达式
            // 计算维度大小（这里是常量5）
            ast_node * dimExpr = std::any_cast<ast_node *>(visitExpr(dimCtx)); // 访问维度表达式

            if (dimExpr->node_type == AST_OP_LEAF_LITERAL_UINT) { // 检查是否是整数常量
                int dimSize = dimExpr->integer_val;               // 获取维度大小5
                dimensions.push_back(dimSize);                    // 添加到维度向量
            }
        }

        // 创建数组类型
        ArrayType * arrayType = new ArrayType(baseType, dimensions); // 创建浮点数数组类型

        // 创建数组变量节点
        ast_node * arrayNode = create_contain_node(AST_OP_VAR_DECL); // 创建数组声明节点
        arrayNode->name = varName;                                // 设置变量名
        arrayNode->type = arrayType;                              // 设置数组类型

        // 处理数组初始化列表
        if (ctx->initializerList()) {                            // 检查是否有初始化列表
            ast_node * initListNode = std::any_cast<ast_node *>(visitInitializerList(ctx->initializerList()));
            arrayNode->sons.push_back(initListNode);             // 添加初始化列表为子节点
        }

        return arrayNode;                                         // 返回数组声明节点
    }

    // 普通变量声明处理...
}
```

*函数：visitInitializerList() - 处理初始化列表*
```cpp
std::any MiniCCSTVisitor::visitInitializerList(MiniCParser::InitializerListContext * ctx) {
    ast_node * initListNode = create_contain_node(AST_OP_INIT_LIST); // 创建初始化列表节点

    // 遍历初始化列表中的每个表达式
    for (auto exprCtx : ctx->expr()) {                            // 遍历{1.1, 2.2, 3.3, 4.4, 5.5}
        ast_node * exprNode = std::any_cast<ast_node *>(visitExpr(exprCtx)); // 访问每个浮点数表达式
        initListNode->sons.push_back(exprNode);                  // 添加为子节点
    }

    return initListNode;                                          // 返回初始化列表节点
}
```

**阶段2：数组IR生成（IRGenerator.cpp）**

*文件：ir/Generator/IRGenerator.cpp*
*函数：ir_array_decl() - 处理数组声明*
```cpp
bool IRGenerator::ir_array_decl(ast_node * node) {
    std::string arrayName = node->name;                          // 获取数组名"numbers"
    ArrayType * arrayType = static_cast<ArrayType *>(node->type); // 获取数组类型
    Type * elementType = arrayType->getElementType();            // 获取元素类型FloatType
    std::vector<int> dimensions = arrayType->getDimensions();    // 获取维度[5]

    // 创建数组变量
    LocalVariable * arrayVar = new LocalVariable(arrayName, arrayType); // 创建数组变量对象

    // 生成alloca指令分配数组内存
    AllocaInstruction * allocaInst = new AllocaInstruction(      // 创建内存分配指令
        currentFunc,                                              // 当前函数
        arrayVar,                                                 // 数组变量
        arrayType,                                                // 数组类型
        16);                                                      // 16字节对齐（浮点数数组）
    node->blockInsts.addInst(allocaInst);                         // 添加到指令块

    // 处理数组初始化
    if (node->sons.size() > 0 && node->sons[0]->node_type == AST_OP_INIT_LIST) {
        ast_node * initListNode = node->sons[0];                 // 获取初始化列表节点

        // 遍历初始化列表中的每个值
        for (int i = 0; i < initListNode->sons.size(); i++) {    // 遍历5个初始化值
            ast_node * initValueNode = initListNode->sons[i];    // 获取第i个初始化值

            // 生成初始化值的IR
            ir_visit_ast_node(initValueNode);                    // 递归生成浮点数常量IR
            Value * initValue = initValueNode->val;              // 获取初始化值

            // 确保初始化值是浮点数类型
            if (!initValue->getType()->isFloatType()) {          // 检查类型
                initValue = convertToFloat(initValue, currentFunc, node->blockInsts); // 转换为浮点数
            }

            // 生成数组元素地址计算指令
            ConstInt * indexConst = module->newConstInt(0);      // 第一个索引为0
            ConstInt * elemIndexConst = module->newConstInt(i);  // 元素索引

            GetelementptrInstruction * gepInst = new GetelementptrInstruction( // 创建地址计算指令
                currentFunc,                                      // 当前函数
                arrayVar,                                         // 数组基址
                indexConst,                                       // 第一个索引0
                elemIndexConst);                                  // 元素索引i
            node->blockInsts.addInst(gepInst);                    // 添加到指令块

            // 生成store指令存储初始化值
            StoreInstruction * storeInst = new StoreInstruction( // 创建存储指令
                currentFunc,                                      // 当前函数
                initValue,                                        // 要存储的值
                gepInst,                                          // 目标地址
                4);                                               // 4字节对齐
            node->blockInsts.addInst(storeInst);                  // 添加到指令块
        }
    }

    // 注册数组变量到符号表
    module->addLocalVariable(arrayVar);                          // 添加到符号表
    node->val = arrayVar;                                         // 设置节点值

    return true;                                                  // 返回成功
}
```

**阶段3：数组访问处理**

*函数：visitArrayAccess() - 处理数组访问*
```cpp
std::any MiniCCSTVisitor::visitArrayAccess(MiniCParser::ArrayAccessContext * ctx) {
    // 获取数组基址表达式
    ast_node * arrayNode = std::any_cast<ast_node *>(visitPrimaryExpr(ctx->primaryExpr())); // 访问数组名

    // 获取索引表达式
    ast_node * indexNode = std::any_cast<ast_node *>(visitExpr(ctx->expr())); // 访问索引表达式

    // 创建数组访问节点
    ast_node * accessNode = create_contain_node(AST_OP_ARRAY_ACCESS, arrayNode, indexNode); // 创建数组访问节点

    return accessNode;                                            // 返回数组访问节点
}
```

*函数：ir_array_access() - 处理数组访问IR生成*
```cpp
bool IRGenerator::ir_array_access(ast_node * node) {
    ast_node * arrayNode = node->sons[0];                        // 获取数组节点
    ast_node * indexNode = node->sons[1];                        // 获取索引节点

    // 生成数组和索引的IR
    ir_visit_ast_node(arrayNode);                                // 生成数组变量IR
    ir_visit_ast_node(indexNode);                                // 生成索引表达式IR

    Value * arrayVar = arrayNode->val;                           // 获取数组变量
    Value * indexValue = indexNode->val;                         // 获取索引值

    // 生成getelementptr指令计算元素地址
    ConstInt * zeroConst = module->newConstInt(0);               // 第一个索引为0

    GetelementptrInstruction * gepInst = new GetelementptrInstruction( // 创建地址计算指令
        currentFunc,                                              // 当前函数
        arrayVar,                                                 // 数组基址
        zeroConst,                                                // 第一个索引0
        indexValue);                                              // 元素索引
    node->blockInsts.addInst(gepInst);                            // 添加到指令块

    // 生成load指令加载数组元素
    LoadInstruction * loadInst = new LoadInstruction(            // 创建加载指令
        currentFunc,                                              // 当前函数
        gepInst,                                                  // 元素地址
        FloatType::getTypeFloat(),                                // 元素类型
        4);                                                       // 4字节对齐
    node->blockInsts.addInst(loadInst);                           // 添加到指令块

    node->val = loadInst;                                         // 设置节点值为加载指令

    return true;                                                  // 返回成功
}
```

**生成的LLVM IR：**
```llvm
; 数组声明和初始化
%numbers = alloca [5 x float], align 16

; 数组初始化
%1 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 0
store float 1.100000e+00, float* %1, align 16
%2 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 1
store float 2.200000e+00, float* %2, align 4
%3 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 2
store float 3.300000e+00, float* %3, align 8
%4 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 3
store float 4.400000e+00, float* %4, align 4
%5 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 4
store float 5.500000e+00, float* %5, align 16

; 循环中的数组访问
%sum = alloca float, align 4
store float 0.000000e+00, float* %sum, align 4

; for循环体中的数组访问：sum += numbers[i]
%6 = load i32, i32* %i, align 4                    ; 加载索引i
%7 = sext i32 %6 to i64                            ; 扩展索引为64位
%8 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 %7
%9 = load float, float* %8, align 4                ; 加载numbers[i]
%10 = load float, float* %sum, align 4             ; 加载sum
%11 = fadd float %10, %9                           ; 浮点数加法
store float %11, float* %sum, align 4              ; 存储新的sum值
```

## 调用流程总结

### 完整的浮点数处理流程

#### 1. 词法分析阶段
```
源代码: "3.14159"
    ↓
ANTLR4词法分析器 (MiniC.g4)
    ↓
T_FLOAT_DIGIT Token
```

#### 2. 语法分析阶段
```
T_FLOAT_DIGIT Token
    ↓
Antlr4CSTVisitor::visitPrimaryExpression()
    ↓
std::stof() 解析浮点数值
    ↓
创建 AST_OP_LEAF_LITERAL_FLOAT 节点
```

#### 3. 类型系统阶段
```
AST节点
    ↓
FloatType::getTypeFloat() 获取类型
    ↓
类型检查和转换支持
```

#### 4. IR生成阶段
```
AST浮点数节点
    ↓
IRGenerator::ir_leaf_node_float()
    ↓
Module::newConstFloat() 创建常量
    ↓
ConstFloat 对象 + LLVM IR
```

### 关键函数调用链

1. **浮点数解析链**：
   ```
   visitPrimaryExpression() → std::stof() → ast_node::New()
   ```

2. **常量管理链**：
   ```
   ir_leaf_node_float() → newConstFloat() → findConstFloat() → ConstFloat()
   ```

3. **类型转换链**：
   ```
   convertToFloat() → SitofpInstruction() → LLVM IR生成
   ```

4. **运算处理链**：
   ```
   ir_add() → 常量折叠检查 → BinaryInstruction() → LLVM IR生成
   ```

### 示例5：浮点数函数参数和返回值

#### 源代码
```c
float calculate_area(float radius) {
    const float PI = 3.14159;
    return PI * radius * radius;
}

int main() {
    float r = 2.5;
    float area = calculate_area(r);
    return 0;
}
```

#### 详细处理流程

**阶段1：函数定义解析（Antlr4CSTVisitor.cpp）**

*文件：frontend/antlr4/Antlr4CSTVisitor.cpp*
*函数：visitFuncDef() - 处理函数定义*
```cpp
std::any MiniCCSTVisitor::visitFuncDef(MiniCParser::FuncDefContext * ctx) {
    // 获取返回类型
    ast_node * retTypeNode = std::any_cast<ast_node *>(visitType(ctx->type())); // 访问返回类型float

    // 获取函数名
    std::string funcName = ctx->T_ID()->getText();               // 获取函数名"calculate_area"

    // 处理参数列表
    ast_node * paramsNode = nullptr;                             // 初始化参数节点
    if (ctx->formalParams()) {                                   // 检查是否有参数
        paramsNode = std::any_cast<ast_node *>(visitFormalParams(ctx->formalParams())); // 访问参数列表
    }

    // 处理函数体
    ast_node * blockNode = std::any_cast<ast_node *>(visitBlock(ctx->block())); // 访问函数体

    // 创建函数定义节点
    ast_node * funcDefNode = create_contain_node(AST_OP_FUNC_DEF, retTypeNode, paramsNode, blockNode); // 创建函数定义节点
    funcDefNode->name = funcName;                                // 设置函数名

    return funcDefNode;                                          // 返回函数定义节点
}
```

*函数：visitFormalParams() - 处理形参列表*
```cpp
std::any MiniCCSTVisitor::visitFormalParams(MiniCParser::FormalParamsContext * ctx) {
    ast_node * paramsNode = create_contain_node(AST_OP_FUNC_FORMAL_PARAMS); // 创建形参列表节点

    // 遍历每个形参
    for (auto paramCtx : ctx->formalParam()) {                   // 遍历形参列表
        ast_node * paramNode = std::any_cast<ast_node *>(visitFormalParam(paramCtx)); // 访问每个形参
        paramsNode->sons.push_back(paramNode);                   // 添加为子节点
    }

    return paramsNode;                                           // 返回形参列表节点
}
```

*函数：visitFormalParam() - 处理单个形参*
```cpp
std::any MiniCCSTVisitor::visitFormalParam(MiniCParser::FormalParamContext * ctx) {
    // 获取参数类型
    ast_node * typeNode = std::any_cast<ast_node *>(visitType(ctx->type())); // 访问参数类型float

    // 获取参数名
    std::string paramName = ctx->T_ID()->getText();              // 获取参数名"radius"

    // 创建形参节点
    ast_node * paramNode = create_contain_node(AST_OP_FUNC_FORMAL_PARAM, typeNode); // 创建形参节点
    paramNode->name = paramName;                                 // 设置参数名
    paramNode->type = typeNode->type;                            // 设置参数类型

    return paramNode;                                            // 返回形参节点
}
```

**阶段2：函数IR生成（IRGenerator.cpp）**

*文件：ir/Generator/IRGenerator.cpp*
*函数：ir_function_define() - 处理函数定义*
```cpp
bool IRGenerator::ir_function_define(ast_node * node) {
    // 获取返回类型
    ast_node * retTypeNode = node->sons[0];                      // 获取返回类型节点
    Type * retType = retTypeNode->type;                          // 获取返回类型FloatType

    // 获取函数名
    std::string funcName = node->name;                           // 获取函数名"calculate_area"

    // 创建函数对象
    Function * func = new Function(funcName, retType);           // 创建函数对象

    // 处理参数列表
    if (node->sons[1]) {                                         // 检查是否有参数
        ast_node * paramsNode = node->sons[1];                   // 获取参数列表节点

        for (auto paramNode : paramsNode->sons) {                // 遍历每个参数
            std::string paramName = paramNode->name;             // 获取参数名"radius"
            Type * paramType = paramNode->type;                  // 获取参数类型FloatType

            // 创建形参对象
            FormalParam * param = new FormalParam(paramName, paramType); // 创建形参对象
            func->addParam(param);                               // 添加到函数参数列表

            // 为浮点数参数分配栈空间
            if (paramType->isFloatType()) {                      // 检查是否是浮点数参数
                AllocaInstruction * allocaInst = new AllocaInstruction( // 创建内存分配指令
                    func,                                        // 当前函数
                    param,                                       // 参数对象
                    paramType,                                   // 参数类型
                    4);                                          // 4字节对齐
                func->getInterCode().addInst(allocaInst);        // 添加到函数指令列表

                // 将参数值存储到栈空间
                StoreInstruction * storeInst = new StoreInstruction( // 创建存储指令
                    func,                                        // 当前函数
                    param,                                       // 参数值
                    param,                                       // 目标地址（参数本身）
                    4);                                          // 4字节对齐
                func->getInterCode().addInst(storeInst);         // 添加到函数指令列表
            }
        }
    }

    // 注册函数到模块
    module->addFunction(func);                                   // 将函数添加到模块
    module->setCurrentFunction(func);                            // 设置为当前函数
    currentFunc = func;                                          // 更新当前函数指针

    // 处理函数体
    ast_node * blockNode = node->sons[2];                        // 获取函数体节点
    ir_visit_ast_node(blockNode);                                // 递归处理函数体

    // 将函数体指令添加到函数
    func->getInterCode().addInterCode(blockNode->blockInsts);    // 添加函数体指令

    return true;                                                 // 返回成功
}
```

**阶段3：函数调用处理**

*函数：visitFuncCall() - 处理函数调用*
```cpp
std::any MiniCCSTVisitor::visitFuncCall(MiniCParser::FuncCallContext * ctx) {
    // 获取函数名
    std::string funcName = ctx->T_ID()->getText();               // 获取函数名"calculate_area"

    // 处理实参列表
    ast_node * argsNode = nullptr;                               // 初始化实参节点
    if (ctx->actualParams()) {                                   // 检查是否有实参
        argsNode = std::any_cast<ast_node *>(visitActualParams(ctx->actualParams())); // 访问实参列表
    }

    // 创建函数调用节点
    ast_node * callNode = create_contain_node(AST_OP_FUNC_CALL, argsNode); // 创建函数调用节点
    callNode->name = funcName;                                   // 设置函数名

    return callNode;                                             // 返回函数调用节点
}
```

*函数：ir_function_call() - 处理函数调用IR生成*
```cpp
bool IRGenerator::ir_function_call(ast_node * node) {
    std::string funcName = node->name;                           // 获取函数名"calculate_area"
    Function * calledFunc = module->findFunction(funcName);      // 查找被调用的函数

    if (!calledFunc) {                                           // 检查函数是否存在
        printf("Error: Function '%s' not found\n", funcName.c_str()); // 打印错误信息
        return false;                                            // 返回失败
    }

    // 处理实参
    std::vector<Value *> args;                                   // 创建实参向量
    if (node->sons.size() > 0) {                                 // 检查是否有实参
        ast_node * argsNode = node->sons[0];                     // 获取实参列表节点

        for (int i = 0; i < argsNode->sons.size(); i++) {        // 遍历每个实参
            ast_node * argNode = argsNode->sons[i];               // 获取第i个实参

            // 生成实参的IR
            ir_visit_ast_node(argNode);                          // 递归生成实参IR
            Value * argValue = argNode->val;                     // 获取实参值

            // 获取对应形参的类型
            Type * paramType = calledFunc->getParam(i)->getType(); // 获取形参类型

            // 类型转换（如果需要）
            if (paramType->isFloatType() && !argValue->getType()->isFloatType()) {
                // 整数转浮点数
                argValue = convertToFloat(argValue, currentFunc, node->blockInsts); // 转换为浮点数
            } else if (!paramType->isFloatType() && argValue->getType()->isFloatType()) {
                // 浮点数转整数
                argValue = convertToInt(argValue, currentFunc, node->blockInsts); // 转换为整数
            }

            // 生成ARG指令
            ArgInstruction * argInst = new ArgInstruction(currentFunc, argValue); // 创建参数指令
            node->blockInsts.addInst(argInst);                   // 添加到指令块

            args.push_back(argValue);                            // 添加到实参向量
        }
    }

    // 生成函数调用指令
    Type * retType = calledFunc->getReturnType();                // 获取返回类型
    FuncCallInstruction * callInst = new FuncCallInstruction(   // 创建函数调用指令
        currentFunc,                                             // 当前函数
        calledFunc,                                              // 被调用函数
        retType);                                                // 返回类型

    // 添加参数到调用指令
    for (Value * arg : args) {                                   // 遍历实参
        callInst->addOperand(arg);                               // 添加操作数
    }

    node->blockInsts.addInst(callInst);                          // 添加到指令块
    node->val = callInst;                                        // 设置节点值为调用指令

    return true;                                                 // 返回成功
}
```

**阶段4：返回语句处理**

*函数：ir_return() - 处理返回语句*
```cpp
bool IRGenerator::ir_return(ast_node * node) {
    Function * currentFunc = module->getCurrentFunction();       // 获取当前函数
    if (!currentFunc) {                                          // 检查函数是否存在
        printf("Error: Return statement outside function.\n");   // 打印错误信息
        return false;                                            // 返回失败
    }

    Value * returnValue = nullptr;                               // 初始化返回值

    if (node->sons.size() > 0) {                                 // 检查是否有返回值
        // 有返回值
        ast_node * retValueNode = node->sons[0];                 // 获取返回值节点
        ir_visit_ast_node(retValueNode);                         // 递归生成返回值IR
        returnValue = retValueNode->val;                         // 获取返回值

        // 检查返回值类型是否匹配函数返回类型
        Type * funcRetType = currentFunc->getReturnType();       // 获取函数返回类型
        Type * valueType = returnValue->getType();               // 获取返回值类型

        if (!valueType->isSameType(funcRetType)) {               // 检查类型是否匹配
            // 需要类型转换
            if (funcRetType->isFloatType() && valueType->isIntegerType()) {
                // 整数转浮点数
                returnValue = convertToFloat(returnValue, currentFunc, node->blockInsts); // 转换为浮点数
            } else if (funcRetType->isIntegerType() && valueType->isFloatType()) {
                // 浮点数转整数
                returnValue = convertToInt(returnValue, currentFunc, node->blockInsts); // 转换为整数
            }
        }
    }

    // 生成return指令
    ReturnInstruction * retInst = new ReturnInstruction(currentFunc, returnValue); // 创建返回指令
    node->blockInsts.addInst(retInst);                           // 添加到指令块

    return true;                                                 // 返回成功
}
```

**生成的LLVM IR：**
```llvm
; 函数定义：float calculate_area(float radius)
define float @calculate_area(float %radius) {
entry:
  %radius.addr = alloca float, align 4              ; 为参数分配栈空间
  %PI = alloca float, align 4                       ; 为局部常量分配空间
  store float %radius, float* %radius.addr, align 4 ; 存储参数值

  ; 局部常量：const float PI = 3.14159;
  store float 3.141590e+00, float* %PI, align 4

  ; 返回表达式：PI * radius * radius
  %0 = load float, float* %PI, align 4              ; 加载PI
  %1 = load float, float* %radius.addr, align 4     ; 加载radius
  %2 = fmul float %0, %1                            ; PI * radius
  %3 = load float, float* %radius.addr, align 4     ; 再次加载radius
  %4 = fmul float %2, %3                            ; (PI * radius) * radius
  ret float %4                                      ; 返回结果
}

; 主函数
define i32 @main() {
entry:
  %r = alloca float, align 4
  %area = alloca float, align 4

  ; 初始化：float r = 2.5;
  store float 2.500000e+00, float* %r, align 4

  ; 函数调用：float area = calculate_area(r);
  %0 = load float, float* %r, align 4               ; 加载实参
  %1 = call float @calculate_area(float %0)         ; 调用函数
  store float %1, float* %area, align 4             ; 存储返回值

  ret i32 0                                         ; 返回0
}
```

这个完整的浮点数功能实现涵盖了从源代码解析到LLVM IR生成的全过程，确保了浮点数在编译器前端的正确处理和优化。

### 示例6：综合浮点数程序

#### 源代码
```c
// 计算圆的面积和周长
float calculate_circle_area(float radius) {
    const float PI = 3.14159;
    return PI * radius * radius;
}

float calculate_circle_perimeter(float radius) {
    const float PI = 3.14159;
    return 2.0 * PI * radius;
}

int main() {
    float radii[3] = {1.0, 2.5, 5.0};
    float areas[3];
    float perimeters[3];
    float total_area = 0.0;

    for (int i = 0; i < 3; i++) {
        areas[i] = calculate_circle_area(radii[i]);
        perimeters[i] = calculate_circle_perimeter(radii[i]);
        total_area += areas[i];
    }

    float average_area = total_area / 3.0;
    return (int)average_area;
}
```

#### 详细处理流程

**阶段1：多维度浮点数处理**

这个综合示例涵盖了浮点数功能的所有方面：
- 浮点数函数定义和调用
- 浮点数数组声明和初始化
- 浮点数循环和条件控制
- 浮点数运算和类型转换
- 浮点数常量和变量

**阶段2：关键处理步骤**

*步骤1：函数定义处理*
```cpp
// 处理calculate_circle_area函数
bool IRGenerator::ir_function_define(ast_node * node) {
    std::string funcName = node->name;                           // "calculate_circle_area"
    Type * retType = node->sons[0]->type;                        // FloatType返回类型

    Function * func = new Function(funcName, retType);           // 创建函数对象

    // 处理浮点数参数radius
    FormalParam * radiusParam = new FormalParam("radius", FloatType::getTypeFloat()); // 创建浮点数参数
    func->addParam(radiusParam);                                 // 添加参数到函数

    // 为参数分配栈空间
    AllocaInstruction * paramAlloca = new AllocaInstruction(    // 为参数分配内存
        func, radiusParam, FloatType::getTypeFloat(), 4);        // 4字节对齐的浮点数
    func->getInterCode().addInst(paramAlloca);                   // 添加到函数指令列表

    module->addFunction(func);                                   // 注册函数到模块
    return true;                                                 // 返回成功
}
```

*步骤2：数组处理*
```cpp
// 处理浮点数数组声明：float radii[3] = {1.0, 2.5, 5.0};
bool IRGenerator::ir_array_decl(ast_node * node) {
    std::string arrayName = node->name;                          // "radii"
    ArrayType * arrayType = static_cast<ArrayType *>(node->type); // 获取数组类型

    // 创建数组变量
    LocalVariable * arrayVar = new LocalVariable(arrayName, arrayType); // 创建数组变量

    // 分配数组内存
    AllocaInstruction * allocaInst = new AllocaInstruction(      // 分配数组内存
        currentFunc, arrayVar, arrayType, 16);                   // 16字节对齐
    node->blockInsts.addInst(allocaInst);                        // 添加到指令块

    // 处理数组初始化 {1.0, 2.5, 5.0}
    ast_node * initListNode = node->sons[0];                     // 获取初始化列表
    for (int i = 0; i < initListNode->sons.size(); i++) {        // 遍历初始化值
        ast_node * initValueNode = initListNode->sons[i];        // 获取第i个初始化值
        ir_visit_ast_node(initValueNode);                        // 生成初始化值IR

        Value * initValue = initValueNode->val;                  // 获取初始化值

        // 计算数组元素地址
        ConstInt * zeroIndex = module->newConstInt(0);           // 第一个索引0
        ConstInt * elemIndex = module->newConstInt(i);           // 元素索引i

        GetelementptrInstruction * gepInst = new GetelementptrInstruction( // 地址计算
            currentFunc, arrayVar, zeroIndex, elemIndex);        // 计算radii[i]的地址
        node->blockInsts.addInst(gepInst);                       // 添加到指令块

        // 存储初始化值
        StoreInstruction * storeInst = new StoreInstruction(     // 存储指令
            currentFunc, initValue, gepInst, 4);                 // 存储到radii[i]
        node->blockInsts.addInst(storeInst);                     // 添加到指令块
    }

    return true;                                                 // 返回成功
}
```

*步骤3：循环中的函数调用*
```cpp
// 处理循环中的函数调用：areas[i] = calculate_circle_area(radii[i]);
bool IRGenerator::ir_assign_array_element(ast_node * node) {
    ast_node * leftNode = node->sons[0];                        // areas[i]
    ast_node * rightNode = node->sons[1];                       // calculate_circle_area(radii[i])

    // 处理右侧函数调用
    ir_visit_ast_node(rightNode);                               // 生成函数调用IR
    Value * callResult = rightNode->val;                        // 获取函数调用结果

    // 处理左侧数组访问
    ast_node * arrayNode = leftNode->sons[0];                   // areas数组
    ast_node * indexNode = leftNode->sons[1];                   // 索引i

    ir_visit_ast_node(arrayNode);                               // 生成数组变量IR
    ir_visit_ast_node(indexNode);                               // 生成索引IR

    Value * arrayVar = arrayNode->val;                          // 获取数组变量
    Value * indexValue = indexNode->val;                        // 获取索引值

    // 计算数组元素地址
    ConstInt * zeroIndex = module->newConstInt(0);              // 第一个索引0
    GetelementptrInstruction * gepInst = new GetelementptrInstruction( // 地址计算
        currentFunc, arrayVar, zeroIndex, indexValue);          // 计算areas[i]的地址
    node->blockInsts.addInst(gepInst);                          // 添加到指令块

    // 存储函数调用结果到数组元素
    StoreInstruction * storeInst = new StoreInstruction(        // 存储指令
        currentFunc, callResult, gepInst, 4);                   // 存储到areas[i]
    node->blockInsts.addInst(storeInst);                        // 添加到指令块

    return true;                                                // 返回成功
}
```

*步骤4：浮点数除法和类型转换*
```cpp
// 处理：float average_area = total_area / 3.0;
bool IRGenerator::ir_div(ast_node * node) {
    ast_node * leftNode = node->sons[0];                        // total_area
    ast_node * rightNode = node->sons[1];                       // 3.0

    ir_visit_ast_node(leftNode);                                // 生成左操作数IR
    ir_visit_ast_node(rightNode);                               // 生成右操作数IR

    Value * leftValue = leftNode->val;                          // 获取左操作数值
    Value * rightValue = rightNode->val;                        // 获取右操作数值

    // 确保两个操作数都是浮点数
    Value * leftFloat = convertToFloat(leftValue, currentFunc, node->blockInsts);  // 转换左操作数
    Value * rightFloat = convertToFloat(rightValue, currentFunc, node->blockInsts); // 转换右操作数

    // 生成浮点数除法指令
    BinaryInstruction * divInst = new BinaryInstruction(        // 创建除法指令
        currentFunc,                                             // 当前函数
        IRInstOperator::IRINST_OP_DIV_F,                        // 浮点数除法操作码
        leftFloat,                                               // 左操作数
        rightFloat,                                              // 右操作数
        FloatType::getTypeFloat());                              // 结果类型

    node->blockInsts.addInst(divInst);                          // 添加到指令块
    node->val = divInst;                                        // 设置节点值

    return true;                                                // 返回成功
}

// 处理：return (int)average_area;
bool IRGenerator::ir_return_with_cast(ast_node * node) {
    ast_node * castNode = node->sons[0];                        // (int)average_area
    ast_node * exprNode = castNode->sons[1];                    // average_area

    ir_visit_ast_node(exprNode);                                // 生成表达式IR
    Value * exprValue = exprNode->val;                          // 获取表达式值

    // 浮点数转整数
    FptosiInstruction * castInst = new FptosiInstruction(       // 创建类型转换指令
        currentFunc,                                             // 当前函数
        exprValue,                                               // 源值（浮点数）
        IntegerType::getTypeInt());                              // 目标类型（整数）

    node->blockInsts.addInst(castInst);                         // 添加到指令块

    // 生成return指令
    ReturnInstruction * retInst = new ReturnInstruction(currentFunc, castInst); // 创建返回指令
    node->blockInsts.addInst(retInst);                          // 添加到指令块

    return true;                                                // 返回成功
}
```

**生成的LLVM IR（部分）：**
```llvm
; 函数定义
define float @calculate_circle_area(float %radius) {
entry:
  %radius.addr = alloca float, align 4
  %PI = alloca float, align 4
  store float %radius, float* %radius.addr, align 4
  store float 3.141590e+00, float* %PI, align 4

  %0 = load float, float* %PI, align 4
  %1 = load float, float* %radius.addr, align 4
  %2 = fmul float %0, %1
  %3 = fmul float %2, %1
  ret float %3
}

; 主函数
define i32 @main() {
entry:
  ; 数组声明
  %radii = alloca [3 x float], align 16
  %areas = alloca [3 x float], align 16
  %perimeters = alloca [3 x float], align 16
  %total_area = alloca float, align 4
  %i = alloca i32, align 4
  %average_area = alloca float, align 4

  ; 数组初始化
  %0 = getelementptr inbounds [3 x float], [3 x float]* %radii, i64 0, i64 0
  store float 1.000000e+00, float* %0, align 16
  %1 = getelementptr inbounds [3 x float], [3 x float]* %radii, i64 0, i64 1
  store float 2.500000e+00, float* %1, align 4
  %2 = getelementptr inbounds [3 x float], [3 x float]* %radii, i64 0, i64 2
  store float 5.000000e+00, float* %2, align 8

  ; 初始化变量
  store float 0.000000e+00, float* %total_area, align 4
  store i32 0, i32* %i, align 4

  ; 循环体（简化）
  %3 = load i32, i32* %i, align 4
  %4 = sext i32 %3 to i64
  %5 = getelementptr inbounds [3 x float], [3 x float]* %radii, i64 0, i64 %4
  %6 = load float, float* %5, align 4
  %7 = call float @calculate_circle_area(float %6)
  %8 = getelementptr inbounds [3 x float], [3 x float]* %areas, i64 0, i64 %4
  store float %7, float* %8, align 4

  ; 计算平均值
  %9 = load float, float* %total_area, align 4
  %10 = fdiv float %9, 3.000000e+00
  store float %10, float* %average_area, align 4

  ; 类型转换和返回
  %11 = load float, float* %average_area, align 4
  %12 = fptosi float %11 to i32
  ret i32 %12
}
```

## 总结

这个完整的浮点数功能实现展示了编译器前端如何处理复杂的浮点数程序，包括：

1. **多种浮点数操作**：常量、变量、数组、函数参数、返回值
2. **类型系统完整性**：自动类型转换、类型检查、类型安全
3. **优化功能**：常量折叠、常量池管理、内存对齐
4. **错误处理**：完善的错误检测和报告机制
5. **LLVM IR生成**：标准的LLVM IR格式，支持后续优化和代码生成

通过这些详细的示例和逐行注释，可以清楚地理解编译器前端是如何一步步处理浮点数功能的，从词法分析到最终的IR生成，每个阶段都有明确的职责和实现方式。

## 错误处理和边界情况

### 1. 浮点数解析错误处理

```cpp
// Antlr4CSTVisitor.cpp:491-496
try {                                                            // 开始异常处理块
    val = std::stof(floatText);                                  // 使用标准库解析浮点数字符串
} catch (const std::exception & e) {                            // 捕获解析过程中的异常
    printf("Error: Failed to parse float '%s' at line %ld: %s\n", // 打印详细错误信息
           floatText.c_str(), lineNo, e.what());                // 包含原始文本、行号和异常描述
    return nullptr;                                              // 返回空指针，停止编译过程
}                                                                // 异常处理块结束
```

**处理的错误情况：**
- 浮点数溢出：`1e1000`
- 格式错误：`1.2.3`
- 无效字符：`1.2abc`

### 2. 类型转换失败处理

```cpp
// IRGenerator.cpp:1517-1518, 1534-1535
printf("Error: Cannot convert value to float type\n");          // 打印浮点数转换错误信息
return nullptr;                                                  // 返回空指针表示转换失败

printf("Error: Cannot convert value to integer type\n");        // 打印整数转换错误信息
return nullptr;                                                  // 返回空指针表示转换失败
```

### 3. 浮点数精度处理

```cpp
// ConstFloat.h:63-69
static std::string formatFloat(float val) {                     // 静态方法，格式化浮点数为字符串
    char buffer[32];                                             // 创建32字节的字符缓冲区
    // 使用科学计数法格式，保证LLVM IR兼容性
    snprintf(buffer, sizeof(buffer), "%.6e", val);              // 使用科学计数法格式化，保留6位有效数字
    return buffer;                                               // 返回格式化后的字符串
}                                                                // 方法结束
```

## 性能优化特性

### 1. 常量折叠优化

```cpp
// IRGenerator.cpp:560-568
// 编译时计算常量表达式
if (leftConstFloat || rightConstFloat) {                        // 检查是否有浮点数常量参与运算
    float leftVal = leftConstFloat ? leftConstFloat->getVal() : // 获取左操作数的浮点数值
                   static_cast<float>(leftConstInt->getVal());  // 或将整数常量转换为浮点数
    float rightVal = rightConstFloat ? rightConstFloat->getVal() : // 获取右操作数的浮点数值
                    static_cast<float>(rightConstInt->getVal()); // 或将整数常量转换为浮点数
    float result = leftVal + rightVal;                           // 在编译时直接计算加法结果

    ConstFloat * resultConst = module->newConstFloat(result);    // 创建结果常量对象
    node->val = resultConst;                                     // 将结果常量赋给AST节点
    return true;                                                 // 返回true表示成功进行了常量折叠
}                                                                // if语句结束
```

**优化效果：**
- `3.14 + 2.71` → 直接计算为 `5.85`
- `PI * 2.0` → 编译时计算，运行时无需计算

### 2. 常量池优化

```cpp
// Module.cpp:221-234
ConstFloat * Module::newConstFloat(float floatVal) {             // 创建或获取浮点数常量的方法
    // 查找是否已存在相同常量，避免重复创建
    ConstFloat * val = findConstFloat(floatVal);                 // 在常量池中查找相同值的常量
    if (!val) {                                                  // 如果没有找到相同的常量
        val = new ConstFloat(floatVal);                          // 创建新的ConstFloat对象
        insertConstFloatDirectly(val);                           // 将新常量插入到常量表中
        constFloatVector.push_back(val);                         // 将新常量添加到常量向量中
    }                                                            // if语句结束
    return val;                                                  // 返回找到的或新创建的常量指针
}                                                                // 方法结束
```

**优化效果：**
- 相同值的浮点数常量只创建一次
- 减少内存使用和对象创建开销
- 提高编译速度

### 3. 智能类型提升

```cpp
// 自动选择最合适的运算类型
if (leftType->isFloatType() || rightType->isFloatType()) {      // 检查是否有浮点数类型参与运算
    // 有浮点数参与，提升为浮点数运算
    Value * leftFloat = convertToFloat(leftValue, currentFunc, node->blockInsts);  // 将左操作数转换为浮点数
    Value * rightFloat = convertToFloat(rightValue, currentFunc, node->blockInsts); // 将右操作数转换为浮点数
    // 使用浮点数指令进行运算
} else {                                                         // 否则
    // 纯整数运算，使用整数指令
}                                                                // if-else语句结束
```

## 总结

编译器的浮点数功能实现具有以下特点：

1. **完整性**：支持从词法分析到LLVM IR生成的完整流程
2. **标准兼容**：遵循IEEE 754标准和LLVM IR规范
3. **性能优化**：包含常量折叠、常量池等编译时优化
4. **错误处理**：完善的错误检测和报告机制
5. **类型安全**：严格的类型检查和自动类型转换
6. **可扩展性**：模块化设计，易于扩展和维护

### 关键技术特性

1. **词法分析**：支持多种浮点数格式（标准小数、科学计数法）
2. **语法分析**：完整的CST到AST转换，保留源代码信息
3. **类型系统**：单例模式的FloatType，支持类型检查和转换
4. **常量管理**：高效的常量池，避免重复创建相同常量
5. **IR生成**：标准的LLVM IR格式，支持后续优化
6. **优化功能**：常量折叠、类型提升、内存对齐等

### 支持的浮点数操作

1. **基本操作**：声明、初始化、赋值、运算
2. **数组操作**：一维/多维数组的声明、初始化、访问
3. **函数操作**：参数传递、返回值、局部变量
4. **类型转换**：与整数的隐式/显式转换
5. **常量操作**：const声明、常量折叠优化

通过这套完整的浮点数处理系统，编译器前端能够正确、高效地处理各种浮点数相关的编程场景，为后续的优化和代码生成奠定了坚实的基础。
