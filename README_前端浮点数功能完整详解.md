# 编译器前端浮点数功能完整详解

## 目录
1. [概述](#概述)
2. [浮点数词法分析](#浮点数词法分析)
3. [浮点数语法分析](#浮点数语法分析)
4. [浮点数类型系统](#浮点数类型系统)
5. [浮点数常量管理](#浮点数常量管理)
6. [浮点数运算](#浮点数运算)
7. [浮点数数组](#浮点数数组)
8. [浮点数函数](#浮点数函数)
9. [浮点数类型转换](#浮点数类型转换)
10. [浮点数IR生成](#浮点数ir生成)
11. [浮点数后端代码生成](#浮点数后端代码生成)
12. [完整示例分析](#完整示例分析)
13. [调用流程总结](#调用流程总结)

## 概述

本编译器的浮点数功能支持完整的IEEE 754标准32位浮点数处理，涵盖浮点数的所有使用场景：

### 支持的浮点数功能
- **浮点数字面量**：十进制、科学计数法（1.23, 1.23e10, .5, 1e-5）
- **浮点数变量**：声明、初始化、赋值
- **浮点数运算**：四则运算（+, -, *, /）、混合运算
- **浮点数数组**：一维、多维数组的声明、初始化、访问
- **浮点数函数**：参数传递、返回值、局部变量
- **浮点数常量**：const声明、常量折叠优化
- **类型转换**：与整数的隐式/显式转换
- **浮点数比较**：关系运算符（==, !=, <, <=, >, >=）

### 涉及的核心文件
```
frontend/antlr4/MiniC.g4                    # 词法语法规则
frontend/antlr4/Antlr4CSTVisitor.cpp        # CST到AST转换
frontend/AST.h/cpp                          # AST节点定义
ir/Types/FloatType.h/cpp                    # 浮点数类型
ir/Values/ConstFloat.h                      # 浮点数常量
ir/Generator/IRGenerator.cpp                # IR生成
ir/Instructions/SitofpInstruction.h/cpp     # 整数转浮点
ir/Instructions/FptosiInstruction.h/cpp     # 浮点转整数
ir/Instructions/BinaryInstruction.h/cpp     # 浮点运算指令
backend/arm64/InstSelectorArm64.cpp         # ARM64指令选择
backend/arm64/ILocArm64.cpp                 # ARM64指令生成
```

## 浮点数词法分析

### 文件位置
- `frontend/antlr4/MiniC.g4` - ANTLR4语法文件

### 核心实现

#### 1. 浮点数Token定义
```antlr
T_FLOAT_DIGIT:                                          // 定义浮点数Token的词法规则名称
    [0-9]+ '.' [0-9]* ([eE] [+-]? [0-9]+)?              // 第一种格式：一个或多个数字 + 小数点 + 零个或多个数字 + 可选的指数部分
    | '.' [0-9]+ ([eE] [+-]? [0-9]+)?                   // 第二种格式：小数点 + 一个或多个数字 + 可选的指数部分
    | [0-9]+ [eE] [+-]? [0-9]+;                         // 第三种格式：一个或多个数字 + 必须的指数部分（e或E + 可选正负号 + 数字）
```

**支持的浮点数格式**：
- **标准小数**：`3.14159`, `2.0`, `0.5`
- **省略整数部分**：`.5`, `.123`, `.5e2`
- **科学计数法**：`1.23e10`, `1e-5`, `2.5E+3`
- **正负指数**：`1.23e+10`, `1.23e-5`

#### 2. 词法优先级和冲突解决
```antlr
// 确保浮点数优先级正确
T_DIGIT: [0-9]+;                                        // 定义整数Token：一个或多个数字（词法优先级较低）
T_FLOAT_DIGIT: [0-9]+ '.' [0-9]*;                       // 定义浮点数Token：数字+小数点+数字（词法优先级较高）
```

**词法分析器行为**：
- `123` → 识别为 `T_DIGIT`（整数）
- `123.` → 识别为 `T_FLOAT_DIGIT`（浮点数）
- `123.45` → 识别为 `T_FLOAT_DIGIT`（浮点数）
- `.45` → 识别为 `T_FLOAT_DIGIT`（浮点数）

## 浮点数语法分析

### 文件位置
- `frontend/antlr4/Antlr4CSTVisitor.cpp` - CST访问器实现
- `frontend/AST.h/cpp` - AST节点定义

### 核心实现

#### 1. 浮点数字面量解析
```cpp
// 位置：Antlr4CSTVisitor.cpp:484-501
std::any MiniCCSTVisitor::visitPrimaryExp(MiniCParser::PrimaryExpContext * ctx) { // 访问主表达式上下文的方法
    ast_node * node = nullptr;                                     // 初始化AST节点指针为空

    if (ctx->T_FLOAT_DIGIT()) {                                    // 检查上下文中是否包含浮点数Token
        // 获取浮点数字面量文本
        std::string floatText = ctx->T_FLOAT_DIGIT()->getText();   // 从Token中提取浮点数的文本表示
        int64_t lineNo = (int64_t) ctx->T_FLOAT_DIGIT()->getSymbol()->getLine(); // 获取Token在源代码中的行号

        // 解析浮点数值
        float val = 0.0f;                                          // 初始化浮点数值为0.0
        try {                                                      // 开始异常处理块
            val = std::stof(floatText);                            // 使用C++标准库将字符串转换为float类型
        } catch (const std::exception & e) {                      // 捕获转换过程中的异常
            printf("Error: Failed to parse float '%s' at line %ld: %s\n",  // 打印详细的错误信息
                   floatText.c_str(), lineNo, e.what());          // 包含原始文本、行号和异常描述
            return nullptr;                                        // 返回空指针表示解析失败
        }                                                          // 异常处理块结束

        // 创建浮点数字面量节点
        node = ast_node::New(floatText, lineNo);                   // 调用AST节点工厂方法创建新节点
        node->float_val = val;                                     // 将解析得到的浮点数值存储到节点中
        node->node_type = ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT; // 设置节点类型为浮点数字面量
    }                                                              // if语句结束

    return node;                                                   // 返回创建的AST节点
}                                                                  // 方法结束
```

#### 2. AST节点数据结构
```cpp
// frontend/AST.h 中的节点定义
class ast_node {                                                  // AST节点类定义
public:                                                           // 公有成员区域
    ast_operator_type node_type;                                  // 节点类型枚举，标识节点的语法类别（如浮点数字面量、运算符等）
    float float_val;                                              // 浮点数值存储字段（仅浮点数字面量节点使用）
    int32_t integer_val;                                          // 整数值存储字段（仅整数字面量节点使用）
    Value * val;                                                  // IR生成阶段产生的Value对象指针
    Type * type;                                                  // 节点的数据类型指针（如FloatType、IntType等）
    int64_t line_no;                                              // 节点对应的源代码行号，用于错误报告和调试
    std::vector<ast_node *> sons;                                 // 子节点指针向量，构成AST的树形结构

    // 条件表达式标签（用于逻辑表达式）
    LabelInstruction * trueLabel;                                 // 条件为真时跳转的标签指令指针
    LabelInstruction * falseLabel;                                // 条件为假时跳转的标签指令指针
};                                                                // 类定义结束
```

#### 3. 浮点数AST节点构造函数
```cpp
// frontend/AST.cpp:53-57
ast_node::ast_node(digit_float_attr attr)                         // 浮点数属性构造函数，接收浮点数属性结构体
    : ast_node(ast_operator_type::AST_OP_LEAF_LITERAL_FLOAT,       // 调用基础构造函数，设置节点类型为浮点数字面量
               FloatType::getTypeFloat(),                          // 设置节点的数据类型为FloatType单例实例
               attr.lineno)                                        // 设置节点的行号信息
{                                                                  // 构造函数体开始
    float_val = attr.val;                                          // 将属性结构体中的浮点数值赋给节点的float_val字段
}                                                                  // 构造函数体结束
```

**调用流程**：
1. ANTLR4词法分析器识别`T_FLOAT_DIGIT`
2. `Antlr4CSTVisitor::visitPrimaryExp()`被调用
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
class FloatType final : public Type {                             // 浮点数类型类，继承自Type基类，final表示不可被继承
public:                                                           // 公有成员区域
    static FloatType * getTypeFloat();                            // 静态方法，单例模式获取全局唯一的浮点类型实例

    std::string toString() const override { return "float"; }     // 重写基类方法，返回类型的字符串表示"float"
    bool isFloatType() { return true; }                           // 重写基类方法，标识这是浮点数类型，返回true
    int32_t getSize() const override { return 4; }               // 重写基类方法，返回浮点数大小（32位=4字节）

    bool isSameType(Type * other) const;                          // 类型比较方法声明，检查是否与另一个类型相同
    bool canConvertTo(Type * target) const;                       // 类型转换检查方法声明，检查是否可转换为目标类型

private:                                                          // 私有成员区域
    FloatType() : Type(Type::FloatTyID) {}                        // 私有构造函数，调用基类构造函数并传入浮点类型ID
    static FloatType * oneInstance;                               // 静态成员变量，存储全局唯一的FloatType实例指针
};                                                                // 类定义结束
```

#### 2. 类型转换支持
```cpp
// 位置：FloatType.cpp:49-52
bool FloatType::canConvertTo(Type * target) const                 // 检查浮点数类型是否可以转换为目标类型
{                                                                 // 函数体开始
    return target->isFloatType() || target->isIntegerType();      // 返回true如果目标类型是浮点数或整数类型
}                                                                 // 函数体结束
```

#### 3. 类型检查函数
```cpp
// Type基类中的虚函数
virtual bool isFloatType() const { return false; }               // 基类中的虚函数，默认返回false表示不是浮点数类型

// FloatType中的重写
bool isFloatType() const override { return true; }               // 在FloatType中重写该方法，返回true表示这是浮点数类型
```

**特性**：
- **单例模式**：全局只有一个FloatType实例，节省内存
- **类型安全**：提供类型检查和转换支持
- **LLVM兼容**：生成标准的LLVM IR类型标识`float`
- **内存对齐**：32位（4字节）对齐

## 浮点数常量管理

### 文件位置
- `ir/Values/ConstFloat.h` - 浮点数常量类定义
- `symboltable/Module.cpp` - 常量管理实现

### 核心实现

#### 1. ConstFloat类设计
```cpp
class ConstFloat final : public Constant {
public:
    explicit ConstFloat(float val) : Constant(FloatType::getTypeFloat()) {
        name = formatFloat(val);  // LLVM IR格式化
        floatVal = val;
    }
    
    std::string getIRName() const override { return name; }
    float getVal() const { return floatVal; }
    
private:
    static std::string formatFloat(float val) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.6e", val);  // 科学计数法
        return buffer;
    }
    
    float floatVal;
    std::string name;
};
```

#### 2. 常量池管理
```cpp
// 位置：Module.cpp:221-234
ConstFloat * Module::newConstFloat(float floatVal)
{
    // 查找是否已存在相同常量
    ConstFloat * val = findConstFloat(floatVal);
    if (!val) {
        // 不存在则创建新常量
        val = new ConstFloat(floatVal);
        insertConstFloatDirectly(val);
        constFloatVector.push_back(val);
    }
    return val;
}

ConstFloat * Module::findConstFloat(float floatVal)
{
    for (auto val : constFloatVector) {
        if (val->getVal() == floatVal) {
            return val;
        }
    }
    return nullptr;
}
```

#### 3. 常量格式化
```cpp
// LLVM IR格式化示例
3.14159f  → "3.141590e+00"
2.5f      → "2.500000e+00"
0.1f      → "1.000000e-01"
1000.0f   → "1.000000e+03"
```

**特性**：
- **常量池优化**：相同值的浮点数常量只创建一次
- **LLVM格式**：自动格式化为LLVM IR要求的科学计数法格式
- **内存管理**：统一管理所有浮点数常量的生命周期
- **精度保证**：使用6位有效数字的科学计数法

## 浮点数运算

### 文件位置
- `ir/Generator/IRGenerator.cpp` - IR生成器实现
- `ir/Instructions/BinaryInstruction.h/cpp` - 二元运算指令

### 核心实现

#### 1. 常量折叠优化
```cpp
// 位置：IRGenerator.cpp:560-568
bool IRGenerator::tryConstantFolding(ast_node * node) {
    Value * leftValue = node->sons[0]->val;
    Value * rightValue = node->sons[1]->val;
    
    ConstFloat * leftConstFloat = dynamic_cast<ConstFloat *>(leftValue);
    ConstFloat * rightConstFloat = dynamic_cast<ConstFloat *>(rightValue);
    ConstInt * leftConstInt = dynamic_cast<ConstInt *>(leftValue);
    ConstInt * rightConstInt = dynamic_cast<ConstInt *>(rightValue);

    // 如果任一操作数是浮点数，结果为浮点数
    if (leftConstFloat || rightConstFloat) {
        float leftVal = leftConstFloat ? leftConstFloat->getVal() : 
                       static_cast<float>(leftConstInt->getVal());
        float rightVal = rightConstFloat ? rightConstFloat->getVal() : 
                        static_cast<float>(rightConstInt->getVal());
        
        float result;
        switch (node->node_type) {
            case AST_OP_ADD: result = leftVal + rightVal; break;
            case AST_OP_SUB: result = leftVal - rightVal; break;
            case AST_OP_MUL: result = leftVal * rightVal; break;
            case AST_OP_DIV: result = leftVal / rightVal; break;
            default: return false;
        }

        ConstFloat * resultConst = module->newConstFloat(result);
        node->val = resultConst;
        printf("Debug: Constant folding result: %f op %f = %f\n", 
               leftVal, rightVal, result);
        return true;
    }
    return false;
}
```

#### 2. 运行时浮点数运算
```cpp
// 浮点数运算指令生成
bool IRGenerator::ir_add(ast_node * node) {
    // 先尝试常量折叠
    if (tryConstantFolding(node)) {
        return true;
    }
    
    Value * leftValue = node->sons[0]->val;
    Value * rightValue = node->sons[1]->val;
    Type * leftType = leftValue->getType();
    Type * rightType = rightValue->getType();
    
    // 检查是否有浮点数参与运算
    if (leftType->isFloatType() || rightType->isFloatType()) {
        // 确保两个操作数都是浮点数类型
        Value * leftFloat = convertToFloat(leftValue, currentFunc, node->blockInsts);
        Value * rightFloat = convertToFloat(rightValue, currentFunc, node->blockInsts);
        
        // 生成浮点数加法指令
        Instruction * addInst = new BinaryInstruction(currentFunc, 
            IRInstOperator::IRINST_OP_ADD_F, leftFloat, rightFloat, FloatType::getTypeFloat());
        node->blockInsts.addInst(addInst);
        node->val = addInst;
    } else {
        // 纯整数运算
        Instruction * addInst = new BinaryInstruction(currentFunc, 
            IRInstOperator::IRINST_OP_ADD_I, leftValue, rightValue, IntegerType::getTypeInt());
        node->blockInsts.addInst(addInst);
        node->val = addInst;
    }
    
    return true;
}
```

#### 3. 浮点数运算指令映射
```cpp
// BinaryInstruction.cpp 中的指令映射
switch (getOp()) {
    case IRInstOperator::IRINST_OP_ADD_F:
        opStr = "fadd";
        break;
    case IRInstOperator::IRINST_OP_SUB_F:
        opStr = "fsub";
        break;
    case IRInstOperator::IRINST_OP_MUL_F:
        opStr = "fmul";
        break;
    case IRInstOperator::IRINST_OP_DIV_F:
        opStr = "fdiv";
        break;
}
```

**支持的运算**：
- **四则运算**：`+`, `-`, `*`, `/`
- **常量折叠**：编译时计算常量表达式
- **类型提升**：整数自动提升为浮点数参与运算
- **混合运算**：支持整数和浮点数混合运算
- **精度保持**：运算结果保持浮点数精度

## 浮点数数组

### 文件位置
- `ir/Types/ArrayType.h/cpp` - 数组类型定义
- `ir/Generator/IRGenerator.cpp` - 数组IR生成
- `ir/Instructions/GetelementptrInstruction.h/cpp` - 数组索引指令

### 核心实现

#### 1. 浮点数数组类型定义
```cpp
// ArrayType支持浮点数元素
class ArrayType : public Type {
private:
    Type * elementType;              // 元素类型（可以是FloatType）
    std::vector<int> dimensions;     // 数组维度

public:
    ArrayType(Type * _elementType, std::vector<int> _dimensions)
        : Type(Type::ArrayTyID), elementType(_elementType), dimensions(_dimensions) {}

    Type * getElementType() const { return elementType; }
    std::vector<int> getDimensions() const { return dimensions; }

    // 计算数组总大小
    int32_t getSize() const override {
        int32_t size = elementType->getSize();
        for (int dim : dimensions) {
            size *= dim;
        }
        return size;
    }
};
```

#### 2. 浮点数数组声明处理
```cpp
// 一维浮点数数组：float arr[10];
// 多维浮点数数组：float matrix[3][4];

// AST处理
std::any MiniCCSTVisitor::visitVarDecl(MiniCParser::VarDeclContext * ctx) {
    // 获取基本类型
    Type * baseType = getTypeFromContext(ctx->type());  // FloatType

    // 处理数组维度
    if (ctx->arrayDimensions()) {
        std::vector<int> dimensions;
        for (auto dimCtx : ctx->arrayDimensions()->expr()) {
            // 计算维度大小
            int dimSize = evaluateConstantExpression(dimCtx);
            dimensions.push_back(dimSize);
        }

        // 创建数组类型
        ArrayType * arrayType = new ArrayType(baseType, dimensions);

        // 创建数组变量节点
        ast_node * arrayNode = create_array_node(arrayType, varName);
        return arrayNode;
    }
}
```

#### 3. 浮点数数组初始化
```cpp
// 数组初始化：float arr[3] = {1.1, 2.2, 3.3};

bool IRGenerator::ir_array_init(ast_node * node) {
    ArrayType * arrayType = static_cast<ArrayType *>(node->type);
    Type * elementType = arrayType->getElementType();

    if (elementType->isFloatType()) {
        // 浮点数数组初始化
        for (int i = 0; i < node->sons.size(); i++) {
            ast_node * initValue = node->sons[i];

            // 确保初始化值是浮点数
            Value * floatValue;
            if (initValue->node_type == AST_OP_LEAF_LITERAL_FLOAT) {
                floatValue = module->newConstFloat(initValue->float_val);
            } else if (initValue->node_type == AST_OP_LEAF_LITERAL_UINT) {
                // 整数转浮点数
                floatValue = module->newConstFloat(static_cast<float>(initValue->integer_val));
            } else {
                // 表达式求值
                ir_visit_ast_node(initValue);
                floatValue = convertToFloat(initValue->val, currentFunc, node->blockInsts);
            }

            // 生成数组元素存储指令
            ConstInt * indexConst = module->newConstInt(i);
            GetelementptrInstruction * gepInst = new GetelementptrInstruction(
                currentFunc, arrayVar, module->newConstInt(0), indexConst);
            node->blockInsts.addInst(gepInst);

            StoreInstruction * storeInst = new StoreInstruction(
                currentFunc, floatValue, gepInst, 4);
            node->blockInsts.addInst(storeInst);
        }
    }

    return true;
}
```

#### 4. 浮点数数组访问
```cpp
// 数组访问：arr[i] 或 matrix[i][j]

bool IRGenerator::ir_array_access(ast_node * node) {
    ast_node * arrayNode = node->sons[0];  // 数组基址
    ast_node * indexNode = node->sons[1];  // 索引表达式

    Value * arrayVar = arrayNode->val;
    Value * indexValue = indexNode->val;

    // 生成getelementptr指令
    GetelementptrInstruction * gepInst;

    if (arrayVar->getType()->isArrayType()) {
        // 直接数组访问
        const ArrayType * arrayType = static_cast<const ArrayType *>(arrayVar->getType());

        if (arrayType->getDimensions().size() > 1) {
            // 多维数组：第一次访问返回子数组的指针
            ConstInt * zeroConst = module->newConstInt(0);
            gepInst = new GetelementptrInstruction(currentFunc, arrayVar, zeroConst, indexValue);
        } else {
            // 一维数组：直接访问元素
            ConstInt * zeroConst = module->newConstInt(0);
            gepInst = new GetelementptrInstruction(currentFunc, arrayVar, zeroConst, indexValue);
        }
    } else if (arrayVar->getType()->isPointerType()) {
        // 指针类型（多维数组的中间访问结果）
        const PointerType * ptrType = static_cast<const PointerType *>(arrayVar->getType());
        const Type * pointeeType = ptrType->getPointeeType();

        if (pointeeType->isArrayType()) {
            // 指向数组的指针：需要两个索引 [0][index]
            ConstInt * zeroConst = module->newConstInt(0);
            gepInst = new GetelementptrInstruction(currentFunc, arrayVar, zeroConst, indexValue);
        } else {
            // 指向元素的指针：使用单个索引 [index]
            gepInst = new GetelementptrInstruction(currentFunc, arrayVar, indexValue);
        }
    }

    node->blockInsts.addInst(gepInst);
    node->val = gepInst;  // 返回地址，不是值

    return true;
}
```

#### 5. 浮点数数组的LLVM IR生成
```llvm
; 一维浮点数数组声明
%arr = alloca [10 x float], align 16

; 数组初始化
store float 1.100000e+00, float* getelementptr inbounds ([10 x float], [10 x float]* %arr, i64 0, i64 0), align 4
store float 2.200000e+00, float* getelementptr inbounds ([10 x float], [10 x float]* %arr, i64 0, i64 1), align 4

; 数组访问
%arrayidx = getelementptr inbounds [10 x float], [10 x float]* %arr, i64 0, i64 %i
%0 = load float, float* %arrayidx, align 4

; 多维数组声明
%matrix = alloca [3 x [4 x float]], align 16

; 多维数组访问
%arrayidx1 = getelementptr inbounds [3 x [4 x float]], [3 x [4 x float]]* %matrix, i64 0, i64 %i
%arrayidx2 = getelementptr inbounds [4 x float], [4 x float]* %arrayidx1, i64 0, i64 %j
%1 = load float, float* %arrayidx2, align 4
```

## 浮点数函数

### 文件位置
- `ir/Values/FormalParam.h` - 函数形参定义
- `ir/Function.h/cpp` - 函数定义
- `ir/Generator/IRGenerator.cpp` - 函数IR生成

### 核心实现

#### 1. 浮点数函数参数
```cpp
// 函数声明：float add(float a, float b);

bool IRGenerator::ir_function_define(ast_node * node) {
    // 获取返回类型
    ast_node * retTypeNode = node->sons[0];
    Type * retType = retTypeNode->type;  // 可能是FloatType

    // 获取函数名
    std::string funcName = node->name;

    // 创建函数对象
    Function * func = new Function(funcName, retType);

    // 处理参数列表
    if (node->sons[1]) {  // 有参数
        ast_node * paramsNode = node->sons[1];

        for (auto paramNode : paramsNode->sons) {
            std::string paramName = paramNode->name;
            Type * paramType = paramNode->type;  // 可能是FloatType

            // 创建形参对象
            FormalParam * param = new FormalParam(paramName, paramType);
            func->addParam(param);

            // 为浮点数参数分配栈空间
            if (paramType->isFloatType()) {
                AllocaInstruction * allocaInst = new AllocaInstruction(func, param, paramType, 4);
                func->getInterCode().addInst(allocaInst);

                // 将参数值存储到栈空间
                StoreInstruction * storeInst = new StoreInstruction(func, param, param, 4);
                func->getInterCode().addInst(storeInst);
            }
        }
    }

    // 注册函数到模块
    module->addFunction(func);
    module->setCurrentFunction(func);

    return true;
}
```

#### 2. 浮点数函数调用
```cpp
// 函数调用：result = add(1.5, 2.5);

bool IRGenerator::ir_function_call(ast_node * node) {
    std::string funcName = node->name;
    Function * calledFunc = module->findFunction(funcName);

    if (!calledFunc) {
        printf("Error: Function '%s' not found\n", funcName.c_str());
        return false;
    }

    // 处理实参
    std::vector<Value *> args;
    if (node->sons.size() > 0) {
        ast_node * argsNode = node->sons[0];

        for (int i = 0; i < argsNode->sons.size(); i++) {
            ast_node * argNode = argsNode->sons[i];

            // 生成实参的IR
            ir_visit_ast_node(argNode);
            Value * argValue = argNode->val;

            // 获取对应形参的类型
            Type * paramType = calledFunc->getParam(i)->getType();

            // 类型转换（如果需要）
            if (paramType->isFloatType() && !argValue->getType()->isFloatType()) {
                // 整数转浮点数
                argValue = convertToFloat(argValue, currentFunc, node->blockInsts);
            } else if (!paramType->isFloatType() && argValue->getType()->isFloatType()) {
                // 浮点数转整数
                argValue = convertToInt(argValue, currentFunc, node->blockInsts);
            }

            // 生成ARG指令
            ArgInstruction * argInst = new ArgInstruction(currentFunc, argValue);
            node->blockInsts.addInst(argInst);

            args.push_back(argValue);
        }
    }

    // 生成函数调用指令
    Type * retType = calledFunc->getReturnType();
    FuncCallInstruction * callInst = new FuncCallInstruction(currentFunc, calledFunc, retType);

    // 添加参数到调用指令
    for (Value * arg : args) {
        callInst->addOperand(arg);
    }

    node->blockInsts.addInst(callInst);
    node->val = callInst;

    return true;
}
```

#### 3. 浮点数函数返回值
```cpp
// 返回语句：return result;

bool IRGenerator::ir_return(ast_node * node) {
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        printf("Error: Return statement outside function.\n");
        return false;
    }

    Value * returnValue = nullptr;

    if (node->sons.size() > 0) {
        // 有返回值
        ast_node * retValueNode = node->sons[0];
        ir_visit_ast_node(retValueNode);
        returnValue = retValueNode->val;

        // 检查返回值类型是否匹配函数返回类型
        Type * funcRetType = currentFunc->getReturnType();
        Type * valueType = returnValue->getType();

        if (!valueType->isSameType(funcRetType)) {
            // 需要类型转换
            if (funcRetType->isFloatType() && valueType->isIntegerType()) {
                // 整数转浮点数
                returnValue = convertToFloat(returnValue, currentFunc, node->blockInsts);
            } else if (funcRetType->isIntegerType() && valueType->isFloatType()) {
                // 浮点数转整数
                returnValue = convertToInt(returnValue, currentFunc, node->blockInsts);
            }
        }
    }

    // 生成return指令
    ReturnInstruction * retInst = new ReturnInstruction(currentFunc, returnValue);
    node->blockInsts.addInst(retInst);

    return true;
}
```

#### 4. 浮点数函数的LLVM IR生成
```llvm
; 浮点数函数定义
define float @add(float %a, float %b) {
entry:
  %a.addr = alloca float, align 4
  %b.addr = alloca float, align 4
  store float %a, float* %a.addr, align 4
  store float %b, float* %b.addr, align 4

  %0 = load float, float* %a.addr, align 4
  %1 = load float, float* %b.addr, align 4
  %2 = fadd float %0, %1
  ret float %2
}

; 浮点数函数调用
define i32 @main() {
entry:
  %result = alloca float, align 4
  %0 = call float @add(float 1.500000e+00, float 2.500000e+00)
  store float %0, float* %result, align 4
  ret i32 0
}
```

## 浮点数类型转换

### 文件位置
- `ir/Generator/IRGenerator.cpp` - 类型转换实现
- `ir/Instructions/SitofpInstruction.h/cpp` - 整数转浮点数指令
- `ir/Instructions/FptosiInstruction.h/cpp` - 浮点数转整数指令

### 核心实现

#### 1. 整数转浮点数
```cpp
// 位置：IRGenerator.cpp:1504-1519
Value * IRGenerator::convertToFloat(Value * val, Function * func, InterCode & blockInsts)
{
    if (val->getType()->isFloatType()) {
        return val; // 已经是浮点数，无需转换
    }

    if (val->getType()->isIntegerType()) {
        // 整数转浮点数 - 使用 sitofp 指令
        Instruction * convInst = new SitofpInstruction(func, val, FloatType::getTypeFloat());
        blockInsts.addInst(convInst);
        return convInst;
    }

    printf("Error: Cannot convert value to float type\n");
    return nullptr;
}
```

#### 2. 浮点数转整数
```cpp
// 位置：IRGenerator.cpp:1521-1536
Value * IRGenerator::convertToInt(Value * val, Function * func, InterCode & blockInsts)
{
    if (val->getType()->isIntegerType()) {
        return val; // 已经是整数，无需转换
    }

    if (val->getType()->isFloatType()) {
        // 浮点数转整数 - 使用 fptosi 指令
        Instruction * convInst = new FptosiInstruction(func, val, IntegerType::getTypeInt());
        blockInsts.addInst(convInst);
        return convInst;
    }

    printf("Error: Cannot convert value to integer type\n");
    return nullptr;
}
```

#### 3. SitofpInstruction实现
```cpp
// SitofpInstruction.h
class SitofpInstruction : public Instruction {
protected:
    Value * srcValue;  // 源值
    Type * targetType; // 目标类型

public:
    SitofpInstruction(Function * _func, Value * _srcValue, Type * _targetType);
    void toString(std::string & str) override;
};

// SitofpInstruction.cpp
SitofpInstruction::SitofpInstruction(Function * _func, Value * _srcValue, Type * _targetType)
    : Instruction(_func, IRInstOperator::IRINST_OP_SITOFP, _targetType),
      srcValue(_srcValue), targetType(_targetType)
{
    addOperand(_srcValue);
}

void SitofpInstruction::toString(std::string & str)
{
    str = getIRName() + " = sitofp " + srcValue->getType()->toString() + " " +
          srcValue->getIRName() + " to " + targetType->toString();
}
```

#### 4. FptosiInstruction实现
```cpp
// FptosiInstruction.h
class FptosiInstruction : public Instruction {
protected:
    Value * srcValue;  // 源值
    Type * targetType; // 目标类型

public:
    FptosiInstruction(Function * _func, Value * _srcValue, Type * _targetType);
    void toString(std::string & str) override;
};

// FptosiInstruction.cpp
FptosiInstruction::FptosiInstruction(Function * _func, Value * _srcValue, Type * _targetType)
    : Instruction(_func, IRInstOperator::IRINST_OP_FPTOSI, _targetType),
      srcValue(_srcValue), targetType(_targetType)
{
    addOperand(_srcValue);
}

void FptosiInstruction::toString(std::string & str)
{
    str = getIRName() + " = fptosi " + srcValue->getType()->toString() + " " +
          srcValue->getIRName() + " to " + targetType->toString();
}
```

#### 5. 隐式类型转换
```cpp
// 混合运算中的自动类型转换
bool IRGenerator::ir_binary_op(ast_node * node, IRInstOperator intOp, IRInstOperator floatOp) {
    Value * leftValue = node->sons[0]->val;
    Value * rightValue = node->sons[1]->val;
    Type * leftType = leftValue->getType();
    Type * rightType = rightValue->getType();

    // 检查是否需要类型转换
    if (leftType->isFloatType() || rightType->isFloatType()) {
        // 有浮点数参与，提升为浮点数运算
        Value * leftFloat = convertToFloat(leftValue, currentFunc, node->blockInsts);
        Value * rightFloat = convertToFloat(rightValue, currentFunc, node->blockInsts);

        // 生成浮点数运算指令
        Instruction * inst = new BinaryInstruction(currentFunc, floatOp,
                                                  leftFloat, rightFloat, FloatType::getTypeFloat());
        node->blockInsts.addInst(inst);
        node->val = inst;
    } else {
        // 纯整数运算
        Instruction * inst = new BinaryInstruction(currentFunc, intOp,
                                                  leftValue, rightValue, IntegerType::getTypeInt());
        node->blockInsts.addInst(inst);
        node->val = inst;
    }

    return true;
}
```

**转换类型**：
- **sitofp**：有符号整数转浮点数（Signed Integer TO Float Point）
- **fptosi**：浮点数转有符号整数（Float Point TO Signed Integer）
- **隐式转换**：运算时自动进行类型提升
- **显式转换**：强制类型转换（cast表达式）

## 浮点数IR生成

### 文件位置
- `ir/Generator/IRGenerator.cpp` - IR生成主逻辑

### 核心实现

#### 1. 浮点数字面量IR生成
```cpp
// 位置：IRGenerator.cpp:1679-1693
bool IRGenerator::ir_leaf_node_float(ast_node * node)
{
    float value = node->float_val;

    // 新建浮点数常量
    ConstFloat * newConst = module->newConstFloat(value);

    // 设置节点的值
    node->val = newConst;

    return true;
}
```

#### 2. 浮点数变量声明IR生成
```cpp
bool IRGenerator::ir_var_decl(ast_node * node) {
    std::string varName = node->name;
    Type * varType = node->type;

    if (varType->isFloatType()) {
        // 浮点数变量声明
        LocalVariable * localVar = new LocalVariable(varName, varType);

        // 生成alloca指令
        AllocaInstruction * allocaInst = new AllocaInstruction(currentFunc, localVar, varType, 4);
        node->blockInsts.addInst(allocaInst);

        // 如果有初始化值
        if (node->sons.size() > 0) {
            ast_node * initNode = node->sons[0];
            ir_visit_ast_node(initNode);

            Value * initValue = initNode->val;

            // 确保初始化值是浮点数类型
            if (!initValue->getType()->isFloatType()) {
                initValue = convertToFloat(initValue, currentFunc, node->blockInsts);
            }

            // 生成store指令
            StoreInstruction * storeInst = new StoreInstruction(currentFunc, initValue, localVar, 4);
            node->blockInsts.addInst(storeInst);
        }

        // 注册变量到符号表
        module->addLocalVariable(localVar);
        node->val = localVar;
    }

    return true;
}
```

#### 3. 浮点数赋值IR生成
```cpp
bool IRGenerator::ir_assign(ast_node * node) {
    ast_node * leftNode = node->sons[0];   // 左值
    ast_node * rightNode = node->sons[1];  // 右值

    // 生成左值的地址
    ir_visit_ast_node(leftNode);
    Value * leftAddr = leftNode->val;

    // 生成右值
    ir_visit_ast_node(rightNode);
    Value * rightValue = rightNode->val;

    // 类型检查和转换
    Type * leftType = leftAddr->getType();
    if (leftType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(leftType);
        Type * pointeeType = ptrType->getPointeeType();

        if (pointeeType->isFloatType() && !rightValue->getType()->isFloatType()) {
            // 左值是浮点数，右值是整数，需要转换
            rightValue = convertToFloat(rightValue, currentFunc, node->blockInsts);
        } else if (!pointeeType->isFloatType() && rightValue->getType()->isFloatType()) {
            // 左值是整数，右值是浮点数，需要转换
            rightValue = convertToInt(rightValue, currentFunc, node->blockInsts);
        }
    }

    // 生成store指令
    StoreInstruction * storeInst = new StoreInstruction(currentFunc, rightValue, leftAddr, 4);
    node->blockInsts.addInst(storeInst);
    node->val = rightValue;

    return true;
}
```

#### 4. 生成的LLVM IR示例
```llvm
; 浮点数常量
%1 = fadd float 3.141590e+00, 2.718280e+00

; 浮点数变量声明和初始化
%x = alloca float, align 4
store float 1.500000e+00, float* %x, align 4

; 类型转换
%2 = sitofp i32 %0 to float
%3 = fptosi float %1 to i32

; 浮点数运算
%4 = fadd float %2, %3
%5 = fsub float %2, %3
%6 = fmul float %2, %3
%7 = fdiv float %2, %3

; 浮点数数组
%arr = alloca [10 x float], align 16
%arrayidx = getelementptr inbounds [10 x float], [10 x float]* %arr, i64 0, i64 %i
%8 = load float, float* %arrayidx, align 4

; 浮点数函数调用
%9 = call float @calculate(float %x, float %y)
```

**调用流程**：
1. AST遍历到浮点数字面量节点
2. 调用`ir_leaf_node_float()`
3. 从`node->float_val`获取浮点数值
4. 调用`module->newConstFloat()`创建常量
5. 将常量赋值给`node->val`

## 浮点数后端代码生成

### 文件位置
- `backend/arm64/InstSelectorArm64.cpp` - ARM64指令选择
- `backend/arm64/ILocArm64.cpp` - ARM64指令生成

### 核心实现

#### 1. 浮点数常量加载
```cpp
// 位置：ILocArm64.cpp:271-285
void ILocArm64::load_float_imm(int rs_reg_no, float val)
{
    // 将float转换为uint32_t的位模式
    uint32_t bits;
    std::memcpy(&bits, &val, sizeof(float));

    // 先将位模式加载到通用寄存器
    emit("mov", "w" + std::to_string(rs_reg_no + 32), "#" + std::to_string(bits));

    // 然后从通用寄存器移动到浮点寄存器
    emit("fmov", "s" + std::to_string(rs_reg_no), "w" + std::to_string(rs_reg_no + 32));
}
```

#### 2. 浮点数类型转换指令
```cpp
// 位置：InstSelectorArm64.cpp:1005-1014
void InstSelectorArm64::translate_fptosi(Instruction * inst)
{
    Value * src = inst->getOperand(0);
    int src_reg_no = src->getRegId();
    int result_reg_no = inst->getRegId();

    // 使用fcvtzs指令：浮点数转有符号整数（向零舍入）
    iloc.inst("fcvtzs", PlatformArm64::regName[result_reg_no],
              PlatformArm64::floatRegName[src_reg_no]);
}

// 位置：InstSelectorArm64.cpp:1018-1027
void InstSelectorArm64::translate_sitofp(Instruction * inst)
{
    Value * src = inst->getOperand(0);
    int src_reg_no = src->getRegId();
    int result_reg_no = inst->getRegId();

    // 使用scvtf指令：有符号整数转浮点数
    iloc.inst("scvtf", PlatformArm64::floatRegName[result_reg_no],
              PlatformArm64::regName[src_reg_no]);
}
```

#### 3. 浮点数运算指令翻译
```cpp
void InstSelectorArm64::translate_binary_float(Instruction * inst) {
    Value * left = inst->getOperand(0);
    Value * right = inst->getOperand(1);
    int left_reg = left->getRegId();
    int right_reg = right->getRegId();
    int result_reg = inst->getRegId();

    std::string left_reg_name = PlatformArm64::floatRegName[left_reg];
    std::string right_reg_name = PlatformArm64::floatRegName[right_reg];
    std::string result_reg_name = PlatformArm64::floatRegName[result_reg];

    switch (inst->getOp()) {
        case IRInstOperator::IRINST_OP_ADD_F:
            iloc.inst("fadd", result_reg_name, left_reg_name, right_reg_name);
            break;
        case IRInstOperator::IRINST_OP_SUB_F:
            iloc.inst("fsub", result_reg_name, left_reg_name, right_reg_name);
            break;
        case IRInstOperator::IRINST_OP_MUL_F:
            iloc.inst("fmul", result_reg_name, left_reg_name, right_reg_name);
            break;
        case IRInstOperator::IRINST_OP_DIV_F:
            iloc.inst("fdiv", result_reg_name, left_reg_name, right_reg_name);
            break;
    }
}
```

#### 4. 浮点数内存操作
```cpp
// 浮点数load指令
void InstSelectorArm64::translate_load_float(Instruction * inst) {
    Value * addr = inst->getOperand(0);
    int result_reg = inst->getRegId();

    std::string result_reg_name = PlatformArm64::floatRegName[result_reg];

    // 生成ldr指令加载浮点数
    iloc.inst("ldr", result_reg_name, "[" + addr->getIRName() + "]");
}

// 浮点数store指令
void InstSelectorArm64::translate_store_float(Instruction * inst) {
    Value * value = inst->getOperand(0);
    Value * addr = inst->getOperand(1);
    int value_reg = value->getRegId();

    std::string value_reg_name = PlatformArm64::floatRegName[value_reg];

    // 生成str指令存储浮点数
    iloc.inst("str", value_reg_name, "[" + addr->getIRName() + "]");
}
```

#### 5. ARM64浮点数寄存器管理
```cpp
// backend/arm64/PlatformArm64.h 中的寄存器定义
class PlatformArm64 {
public:
    // 浮点数寄存器名称映射
    static std::string floatRegName[32];  // s0-s31

    // 通用寄存器名称映射
    static std::string regName[64];       // w0-w31, x0-x31

    // 寄存器分配策略
    // s0-s7:   函数参数和返回值
    // s8-s15:  临时寄存器
    // s16-s31: 保存寄存器
};
```

#### 6. 生成的ARM64指令示例
```assembly
; 浮点数常量加载
mov w10, #0x40490fdb    ; 3.14159的位模式
fmov s0, w10            ; 移动到浮点寄存器

; 浮点数运算
fadd s0, s1, s2         ; 浮点数加法
fsub s0, s1, s2         ; 浮点数减法
fmul s0, s1, s2         ; 浮点数乘法
fdiv s0, s1, s2         ; 浮点数除法

; 类型转换
scvtf s0, w1            ; 整数转浮点数
fcvtzs w0, s1           ; 浮点数转整数

; 内存操作
ldr s0, [sp, #offset]   ; 从内存加载浮点数
str s0, [sp, #offset]   ; 存储浮点数到内存

; 浮点数数组访问
add x1, sp, #array_offset
ldr s0, [x1, x2, lsl #2]  ; 加载arr[i]，每个float 4字节
```

## 完整示例分析

### 示例1：浮点数常量和基本运算

#### 源代码
```c
const float PI = 3.14159;
const float E = 2.71828;

float calculate() {
    float radius = 2.5;
    float area = PI * radius * radius;
    float result = area + E;
    return result;
}
```

#### 详细处理流程

**1. 词法分析阶段**
```
源代码: "3.14159"
    ↓ [MiniC.g4词法规则]
T_FLOAT_DIGIT Token
    ↓
ANTLR4 TokenStream: [T_CONST, T_FLOAT, T_ID(PI), T_ASSIGN, T_FLOAT_DIGIT(3.14159), ...]
```

**2. 语法分析阶段**
```cpp
// Antlr4CSTVisitor.cpp:visitPrimaryExp()
std::string floatText = "3.14159";
float val = std::stof(floatText);  // 3.14159f
ast_node * node = ast_node::New(floatText, lineNo);
node->float_val = val;
node->node_type = AST_OP_LEAF_LITERAL_FLOAT;
```

**3. AST构建**
```
AST_OP_FUNC_DEF (calculate)
├── AST_OP_LEAF_TYPE (float)
└── AST_OP_BLOCK
    ├── AST_OP_VAR_DECL (radius)
    │   └── AST_OP_LEAF_LITERAL_FLOAT (2.5)
    ├── AST_OP_VAR_DECL (area)
    │   └── AST_OP_MUL
    │       ├── AST_OP_MUL
    │       │   ├── AST_OP_LEAF_VAR_ID (PI)
    │       │   └── AST_OP_LEAF_VAR_ID (radius)
    │       └── AST_OP_LEAF_VAR_ID (radius)
    └── AST_OP_RETURN
        └── AST_OP_ADD
            ├── AST_OP_LEAF_VAR_ID (area)
            └── AST_OP_LEAF_VAR_ID (E)
```

**4. IR生成阶段**
```cpp
// 常量处理
ConstFloat * piConst = module->newConstFloat(3.14159f);    // "3.141590e+00"
ConstFloat * eConst = module->newConstFloat(2.71828f);     // "2.718280e+00"
ConstFloat * radiusConst = module->newConstFloat(2.5f);    // "2.500000e+00"

// 常量折叠优化
// PI * radius * radius 在编译时计算
float temp1 = 3.14159f * 2.5f;      // 7.853975f
float temp2 = temp1 * 2.5f;         // 19.634937f
ConstFloat * areaConst = module->newConstFloat(temp2);

// area + E 在编译时计算
float finalResult = 19.634937f + 2.71828f;  // 22.353217f
ConstFloat * resultConst = module->newConstFloat(finalResult);
```

**5. 生成的LLVM IR**
```llvm
define float @calculate() {
entry:
  %radius = alloca float, align 4
  %area = alloca float, align 4
  %result = alloca float, align 4

  ; 由于常量折叠，直接存储计算结果
  store float 2.500000e+00, float* %radius, align 4
  store float 1.963494e+01, float* %area, align 4    ; PI * radius * radius
  store float 2.235322e+01, float* %result, align 4  ; area + E

  %0 = load float, float* %result, align 4
  ret float %0
}
```

**6. ARM64汇编生成**
```assembly
calculate:
    ; 函数序言
    sub sp, sp, #16

    ; 加载常量到浮点寄存器
    mov w0, #0x40200000     ; 2.5的位模式
    fmov s0, w0
    str s0, [sp, #12]       ; 存储radius

    mov w0, #0x419d3f7d     ; 19.634937的位模式
    fmov s0, w0
    str s0, [sp, #8]        ; 存储area

    mov w0, #0x41b2d4fe     ; 22.353217的位模式
    fmov s0, w0
    str s0, [sp, #4]        ; 存储result

    ; 加载返回值
    ldr s0, [sp, #4]

    ; 函数尾声
    add sp, sp, #16
    ret
```

### 示例2：浮点数数组操作

#### 源代码
```c
float sum_array(float arr[], int size) {
    float sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    float numbers[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
    float total = sum_array(numbers, 5);
    return 0;
}
```

#### 详细处理流程

**1. 数组声明处理**
```cpp
// 浮点数数组类型创建
std::vector<int> dimensions = {5};
ArrayType * arrayType = new ArrayType(FloatType::getTypeFloat(), dimensions);

// 数组初始化处理
for (int i = 0; i < 5; i++) {
    ConstFloat * initValue = module->newConstFloat(initValues[i]);
    // 生成getelementptr + store指令
}
```

**2. 数组访问处理**
```cpp
// arr[i] 的处理
GetelementptrInstruction * gepInst = new GetelementptrInstruction(
    currentFunc, arrayVar, module->newConstInt(0), indexValue);
LoadInstruction * loadInst = new LoadInstruction(currentFunc, gepInst, FloatType::getTypeFloat());
```

**3. 生成的LLVM IR**
```llvm
define float @sum_array(float* %arr, i32 %size) {
entry:
  %sum = alloca float, align 4
  %i = alloca i32, align 4
  store float 0.000000e+00, float* %sum, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:
  %0 = load i32, i32* %i, align 4
  %1 = icmp slt i32 %0, %size
  br i1 %1, label %for.body, label %for.end

for.body:
  %2 = load i32, i32* %i, align 4
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds float, float* %arr, i64 %3
  %5 = load float, float* %4, align 4
  %6 = load float, float* %sum, align 4
  %7 = fadd float %6, %5
  store float %7, float* %sum, align 4
  %8 = load i32, i32* %i, align 4
  %9 = add nsw i32 %8, 1
  store i32 %9, i32* %i, align 4
  br label %for.cond

for.end:
  %10 = load float, float* %sum, align 4
  ret float %10
}

define i32 @main() {
entry:
  %numbers = alloca [5 x float], align 16
  %total = alloca float, align 4

  ; 数组初始化
  %0 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 0
  store float 1.100000e+00, float* %0, align 16
  %1 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 1
  store float 2.200000e+00, float* %1, align 4
  ; ... 其他元素初始化

  ; 函数调用
  %2 = getelementptr inbounds [5 x float], [5 x float]* %numbers, i64 0, i64 0
  %3 = call float @sum_array(float* %2, i32 5)
  store float %3, float* %total, align 4

  ret i32 0
}
```

### 示例3：浮点数与整数混合运算

#### 源代码
```c
float mixed_calculation(int count, float rate) {
    float result = count * rate;
    int rounded = (int)(result + 0.5);
    return (float)rounded;
}
```

#### 详细处理流程

**1. 混合运算处理**
```cpp
// count * rate 的处理
Value * countValue = countParam;      // int类型
Value * rateValue = rateParam;       // float类型

// 类型提升：int -> float
Value * countFloat = convertToFloat(countValue, currentFunc, node->blockInsts);
// 生成：%1 = sitofp i32 %count to float

// 浮点数乘法
Instruction * mulInst = new BinaryInstruction(currentFunc,
    IRInstOperator::IRINST_OP_MUL_F, countFloat, rateValue, FloatType::getTypeFloat());
// 生成：%2 = fmul float %1, %rate
```

**2. 强制类型转换处理**
```cpp
// (int)(result + 0.5) 的处理
ConstFloat * halfConst = module->newConstFloat(0.5f);
Instruction * addInst = new BinaryInstruction(currentFunc,
    IRInstOperator::IRINST_OP_ADD_F, resultValue, halfConst, FloatType::getTypeFloat());
// 生成：%3 = fadd float %2, 5.000000e-01

Value * roundedInt = convertToInt(addInst, currentFunc, node->blockInsts);
// 生成：%4 = fptosi float %3 to i32

// (float)rounded 的处理
Value * finalFloat = convertToFloat(roundedInt, currentFunc, node->blockInsts);
// 生成：%5 = sitofp i32 %4 to float
```

**3. 生成的LLVM IR**
```llvm
define float @mixed_calculation(i32 %count, float %rate) {
entry:
  %result = alloca float, align 4
  %rounded = alloca i32, align 4

  ; 类型转换和乘法
  %1 = sitofp i32 %count to float
  %2 = fmul float %1, %rate
  store float %2, float* %result, align 4

  ; 加法和类型转换
  %3 = fadd float %2, 5.000000e-01
  %4 = fptosi float %3 to i32
  store i32 %4, i32* %rounded, align 4

  ; 最终类型转换
  %5 = sitofp i32 %4 to float
  ret float %5
}
```

## 调用流程总结

### 完整的浮点数处理流程

#### 1. 词法分析阶段
```
源代码: "3.14159"
    ↓ [MiniC.g4]
T_FLOAT_DIGIT Token
    ↓ [ANTLR4 Lexer]
Token流
```

#### 2. 语法分析阶段
```
T_FLOAT_DIGIT Token
    ↓ [ANTLR4 Parser]
CST节点
    ↓ [Antlr4CSTVisitor::visitPrimaryExp()]
std::stof() 解析
    ↓
AST_OP_LEAF_LITERAL_FLOAT 节点
```

#### 3. 类型系统阶段
```
AST节点
    ↓ [FloatType::getTypeFloat()]
FloatType实例
    ↓ [类型检查]
类型兼容性验证
```

#### 4. IR生成阶段
```
AST浮点数节点
    ↓ [IRGenerator::ir_leaf_node_float()]
Module::newConstFloat()
    ↓ [常量池管理]
ConstFloat对象
    ↓ [LLVM IR格式化]
"3.141590e+00"
```

#### 5. 后端代码生成阶段
```
LLVM IR
    ↓ [InstSelectorArm64::translate_*()]
ARM64指令选择
    ↓ [ILocArm64::load_float_imm()]
ARM64汇编指令
    ↓ [寄存器分配]
最终机器码
```

### 关键函数调用链

#### 1. 浮点数解析链
```
main()
→ Antlr4Executor::run()
→ MiniCCSTVisitor::visitPrimaryExp()
→ std::stof()
→ ast_node::New()
```

#### 2. 常量管理链
```
IRGenerator::ir_leaf_node_float()
→ Module::newConstFloat()
→ Module::findConstFloat()
→ ConstFloat::ConstFloat()
→ ConstFloat::formatFloat()
```

#### 3. 类型转换链
```
IRGenerator::convertToFloat()
→ SitofpInstruction::SitofpInstruction()
→ InstSelectorArm64::translate_sitofp()
→ ILocArm64::inst("scvtf")
```

#### 4. 运算处理链
```
IRGenerator::ir_add()
→ tryConstantFolding()
→ BinaryInstruction::BinaryInstruction()
→ InstSelectorArm64::translate_binary_float()
→ ILocArm64::inst("fadd")
```

#### 5. 数组处理链
```
IRGenerator::ir_array_access()
→ GetelementptrInstruction::GetelementptrInstruction()
→ InstSelectorArm64::translate_getelementptr()
→ ILocArm64::addr_sp_offset()
```

#### 6. 函数调用链
```
IRGenerator::ir_function_call()
→ ArgInstruction::ArgInstruction()
→ FuncCallInstruction::FuncCallInstruction()
→ InstSelectorArm64::translate_call()
→ ILocArm64::inst("bl")
```

### 文件依赖关系图

```
源代码(.c)
    ↓
MiniC.g4 (词法语法规则)
    ↓
Antlr4CSTVisitor.cpp (CST→AST)
    ↓
AST.h/cpp (AST节点)
    ↓
FloatType.h/cpp (类型系统)
    ↓
IRGenerator.cpp (IR生成)
    ↓
ConstFloat.h (常量管理)
    ↓
BinaryInstruction.cpp (运算指令)
    ↓
SitofpInstruction.cpp (类型转换)
    ↓
InstSelectorArm64.cpp (指令选择)
    ↓
ILocArm64.cpp (指令生成)
    ↓
ARM64汇编(.s)
```

### 性能优化特性

1. **常量折叠**：编译时计算常量表达式，减少运行时计算
2. **常量池**：相同值的浮点数常量只创建一次，节省内存
3. **智能类型提升**：自动选择最优的运算类型
4. **寄存器优化**：合理分配浮点数寄存器，减少内存访问
5. **指令选择**：生成高效的ARM64浮点数指令

### 错误处理机制

1. **词法错误**：无效的浮点数格式
2. **语法错误**：浮点数语法不正确
3. **类型错误**：不兼容的类型转换
4. **精度错误**：浮点数溢出或下溢
5. **运行时错误**：除零等运算错误

通过这套完整的浮点数功能实现，编译器能够正确、高效地处理从简单的浮点数字面量到复杂的浮点数数组和函数调用等各种浮点数相关的编程场景，确保了浮点数在编译器中的完整支持和优化。
```
