# IR数组实现详解

## 1. 概述

本编译器的IR模块实现了完整的数组支持，包括一维和多维数组的声明、初始化、访问等功能。数组实现完全符合LLVM IR规范，支持静态初始化、动态初始化、零初始化等多种初始化方式，并通过GEP（getelementptr）指令实现高效的数组元素访问。

## 2. 核心组件架构

### 2.1 数组类型系统

**核心类型：**
- **`ArrayType`** - 数组类型定义，支持多维数组
- **`PointerType`** - 指针类型，用于数组元素访问
- **`GlobalVariable`** - 全局数组变量
- **`LocalVariable`** - 局部数组变量

**相关文件结构：**
```
ir/Types/
├── ArrayType.h/cpp           # 数组类型实现
├── PointerType.h            # 指针类型定义
└── Type.h                   # 基础类型接口

ir/Instructions/
├── GetelementptrInstruction.h/cpp  # GEP指令实现
├── LoadInstruction.h/cpp           # 加载指令
├── StoreInstruction.h/cpp          # 存储指令
├── AllocaInstruction.h/cpp         # 栈分配指令
├── MemcpyInstruction.h/cpp         # 内存拷贝指令
└── MemsetInstruction.h/cpp         # 内存设置指令
```

### 2.2 数组处理函数

**IR生成器中的数组处理：**
- `ir_array_access()` - 数组访问IR生成
- `ir_array_init()` - 数组初始化IR生成
- `handleStaticInitialization()` - 静态初始化处理
- `handleDynamicInitialization()` - 动态初始化处理
- `handleZeroInitialization()` - 零初始化处理

## 3. 数组类型实现

### 3.1 ArrayType类设计

```cpp
class ArrayType : public Type {
private:
    Type * elementType;          // 数组元素类型
    std::vector<int> dimensions; // 各维度大小

public:
    ArrayType(Type * elementType, std::vector<int> dimensions);
    
    Type * getElementType() const;
    const std::vector<int> & getDimensions() const;
    int getTotalElements() const;
    virtual int32_t getSize() const override;
    virtual std::string toString() const override;
    virtual bool isArrayType() const override;
};
```

### 3.2 多维数组类型表示

#### 3.2.1 维度信息存储

```cpp
ArrayType::ArrayType(Type * elementType, std::vector<int> dimensions)
    : Type(Type::ArrayTyID), elementType(elementType), dimensions(dimensions)
{}

int ArrayType::getTotalElements() const
{
    if (dimensions.empty())
        return 0;
    return std::accumulate(dimensions.begin(), dimensions.end(), 1, std::multiplies<int>());
}

int32_t ArrayType::getSize() const
{
    if (dimensions.empty())
        return 0;
    // 总元素个数乘以单个元素大小
    return getTotalElements() * elementType->getSize();
}
```

#### 3.2.2 LLVM IR格式生成

```cpp
std::string ArrayType::toString() const
{
    if (dimensions.empty()) {
        return elementType->toString();
    }
    
    std::string result = elementType->toString();
    
    // 生成LLVM IR标准格式 [N x type]
    // 从最内层开始构建，生成嵌套的数组类型
    for (int i = dimensions.size() - 1; i >= 0; i--) {
        result = "[" + std::to_string(dimensions[i]) + " x " + result + "]";
    }
    
    return result;
}
```

**示例：**
- 一维数组 `int arr[5]` → `[5 x i32]`
- 二维数组 `int arr[3][4]` → `[3 x [4 x i32]]`
- 三维数组 `int arr[2][3][4]` → `[2 x [3 x [4 x i32]]]`

## 4. 数组访问实现

### 4.1 GEP指令实现

#### 4.1.1 GetelementptrInstruction类

```cpp
class GetelementptrInstruction : public Instruction {
protected:
    Value * basePtr;     // 基础指针（数组变量）
    Value * firstIndex;  // 第一个索引（通常是0）
    Value * secondIndex; // 第二个索引（实际数组索引）
    bool inbounds;       // 是否使用inbounds标记

public:
    // 单索引构造函数
    GetelementptrInstruction(Function * _func, Value * _basePtr, Value * _index, bool _inbounds = true);
    
    // 双索引构造函数
    GetelementptrInstruction(Function * _func, Value * _basePtr, Value * _firstIndex, 
                           Value * _secondIndex, bool _inbounds = true);
    
    void toString(std::string & str) override;
};
```

#### 4.1.2 GEP指令字符串生成

```cpp
void GetelementptrInstruction::toString(std::string & str)
{
    std::string inboundsStr = inbounds ? " inbounds" : "";
    
    if (secondIndex) {
        // 双索引版本：getelementptr inbounds [N x type], [N x type]* %ptr, i32 0, i32 %index
        str = getIRName() + " = getelementptr" + inboundsStr + " " + 
              basePtr->getType()->toString() + ", " + 
              basePtr->getType()->toString() + "* " + basePtr->getIRName() + ", " +
              firstIndex->getType()->toString() + " " + firstIndex->getIRName() + ", " +
              secondIndex->getType()->toString() + " " + secondIndex->getIRName();
    } else {
        // 单索引版本：getelementptr inbounds type, type* %ptr, i32 %index
        str = getIRName() + " = getelementptr" + inboundsStr + " " +
              basePtr->getType()->toString() + ", " +
              basePtr->getType()->toString() + "* " + basePtr->getIRName() + ", " +
              firstIndex->getType()->toString() + " " + firstIndex->getIRName();
    }
}
```

### 4.2 数组访问IR生成

#### 4.2.1 ir_array_access函数

```cpp
bool IRGenerator::ir_array_access(ast_node * node)
{
    if (node->sons.size() != 2) {
        printf("Error: Invalid array access node structure.\n");
        return false;
    }
    
    ast_node * arrayNode = node->sons[0]; // 数组名
    ast_node * indexNode = node->sons[1]; // 索引表达式
    
    // 生成数组基址的IR
    if (!ir_visit_ast_node(arrayNode)) {
        return false;
    }
    node->blockInsts.addInst(arrayNode->blockInsts);
    
    // 生成索引表达式的IR
    if (!ir_visit_ast_node(indexNode)) {
        return false;
    }
    node->blockInsts.addInst(indexNode->blockInsts);
    
    Value * arrayVar = arrayNode->val;
    Value * indexValue = indexNode->val;
    
    // 如果索引需要加载，先加载
    if (needsLoad(indexValue)) {
        LoadInstruction * loadIndex = new LoadInstruction(module->getCurrentFunction(), indexValue, indexValue, 4);
        node->blockInsts.addInst(loadIndex);
        indexValue = loadIndex;
    }
    
    GetelementptrInstruction * gepInst = nullptr;
    
    // 检查arrayVar的类型来决定使用哪种getelementptr格式
    if (arrayVar->getType()->isArrayType()) {
        // 对于多维数组类型，需要检查是否是最后一维访问
        const ArrayType * arrayType = static_cast<const ArrayType *>(arrayVar->getType());
        const std::vector<int> & dimensions = arrayType->getDimensions();
        
        if (dimensions.size() > 1) {
            // 多维数组：第一次访问返回子数组的指针
            ConstInt * zeroConst = module->newConstInt(0);
            gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, zeroConst, indexValue);
        } else {
            // 一维数组：直接访问元素
            ConstInt * zeroConst = module->newConstInt(0);
            gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, zeroConst, indexValue);
        }
    } else if (arrayVar->getType()->isPointerType()) {
        // 对于指针类型（可能是多维数组的中间访问结果）
        const PointerType * ptrType = static_cast<const PointerType *>(arrayVar->getType());
        const Type * pointeeType = ptrType->getPointeeType();
        
        if (pointeeType->isArrayType()) {
            // 指向数组的指针：需要两个索引 [0][index]
            ConstInt * zeroConst = module->newConstInt(0);
            gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, zeroConst, indexValue);
        } else {
            // 指向元素的指针：使用单个索引 [index]
            gepInst = new GetelementptrInstruction(module->getCurrentFunction(), arrayVar, indexValue);
        }
    } else {
        printf("Error: Invalid array base type for getelementptr.\n");
        return false;
    }
    
    node->blockInsts.addInst(gepInst);
    
    // 数组访问的结果是地址，不是值
    node->val = gepInst;
    
    return true;
}
```

#### 4.2.2 多维数组访问示例

**C代码：** `arr[i][j]`

**生成的IR：**
```llvm
; 第一次访问：arr[i]
%1 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %arr, i32 0, i32 %i

; 第二次访问：arr[i][j]  
%2 = getelementptr inbounds [2 x i32], [2 x i32]* %1, i32 0, i32 %j

; 加载值
%3 = load i32, i32* %2, align 4
```

## 5. 数组初始化实现

### 5.1 初始化类型分类

本编译器支持三种数组初始化方式：

1. **静态初始化**：使用常量初始化列表
2. **动态初始化**：使用变量表达式初始化
3. **零初始化**：未显式初始化的数组

### 5.2 静态初始化实现

#### 5.2.1 全局常量数组创建

```cpp
bool IRGenerator::handleStaticInitialization(ast_node * node, Value * arrayVar, ArrayType * arrayType, 
                                            const std::vector<Value *> & initValues)
{
    Function * currentFunc = module->getCurrentFunction();
    
    // 1. 创建全局常量数组
    std::string globalArrayName;
    if (!node->name.empty()) {
        globalArrayName = "__const.main." + node->name;
    }
    
    GlobalVariable * constArray = module->newGlobalConstArray(arrayType, globalArrayName);
    
    // 2. 设置全局数组属性
    constArray->setConstant(true);
    constArray->setBSSSection(false); // 有初值，不在BSS段
    constArray->setAlignment(16);     // 设置16字节对齐
    
    // 3. 处理初始化值
    std::vector<Value *> finalInitValues;
    for (auto value : initValues) {
        if (auto constInt = dynamic_cast<ConstInt *>(value)) {
            finalInitValues.push_back(constInt);
        } else if (auto constFloat = dynamic_cast<ConstFloat *>(value)) {
            finalInitValues.push_back(constFloat);
        } else {
            printf("Error: Global constant array can only be initialized with constant values.\n");
            return false;
        }
    }
    
    // 4. 填充剩余元素为零
    int totalElements = arrayType->getTotalElements();
    while (finalInitValues.size() < totalElements) {
        finalInitValues.push_back(module->newConstInt(0));
    }
    
    constArray->setInitValueList(finalInitValues);
    
    // 5. 生成memcpy指令拷贝到局部数组
    return generateMemcpyForStaticInit(node, arrayVar, constArray, arrayType);
}
```

#### 5.2.2 内存拷贝生成

```cpp
bool IRGenerator::generateMemcpyForStaticInit(ast_node * node, Value * arrayVar, 
                                            GlobalVariable * constArray, ArrayType * arrayType)
{
    Function * currentFunc = module->getCurrentFunction();
    
    // 1. 将局部数组转换为 i8*
    BitcastInstruction * destCast = new BitcastInstruction(currentFunc, arrayVar, module->getI8PtrType());
    node->blockInsts.addInst(destCast);
    
    // 2. 将常量数组转换为 i8*
    BitcastInstruction * srcCast = new BitcastInstruction(currentFunc, constArray, module->getI8PtrType());
    node->blockInsts.addInst(srcCast);
    
    // 3. 计算拷贝大小
    int totalSize = arrayType->getSize();
    ConstInt * sizeConst = module->newConstInt(totalSize);
    
    // 4. 生成 memcpy 指令
    MemcpyInstruction * memcpyInst = new MemcpyInstruction(currentFunc, destCast, srcCast, sizeConst, false);
    node->blockInsts.addInst(memcpyInst);
    
    return true;
}
```

### 5.3 动态初始化实现

#### 5.3.1 逐元素赋值

```cpp
bool IRGenerator::handleDynamicInitialization(ast_node * node, Value * arrayVar, ArrayType * arrayType,
                                             const std::vector<Value *> & initValues)
{
    Function * currentFunc = module->getCurrentFunction();
    const std::vector<int> & dimensions = arrayType->getDimensions();
    
    if (dimensions.size() == 1) {
        // 一维数组的动态初始化
        return handleOneDimensionalDynamicInit(node, arrayVar, arrayType, initValues);
    } else if (dimensions.size() == 2) {
        // 二维数组的动态初始化
        return handleTwoDimensionalDynamicInit(node, arrayVar, arrayType, initValues);
    } else {
        printf("Error: Dynamic initialization for arrays with more than 2 dimensions not supported.\n");
        return false;
    }
}
```

#### 5.3.2 二维数组动态初始化

```cpp
bool IRGenerator::handleTwoDimensionalDynamicInit(ast_node * node, Value * arrayVar, ArrayType * arrayType,
                                                const std::vector<Value *> & initValues)
{
    Function * currentFunc = module->getCurrentFunction();
    const std::vector<int> & dimensions = arrayType->getDimensions();
    int rows = dimensions[0];
    int cols = dimensions[1];
    
    // 按行列顺序填充数组
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // 生成目标地址：arrayVar[row][col]
            ConstInt * rowIndex = module->newConstInt(row);
            ConstInt * colIndex = module->newConstInt(col);
            ConstInt * zeroConst = module->newConstInt(0);
            
            // 首先获取行地址：arrayVar[0][row]
            GetelementptrInstruction * rowGepInst = 
                new GetelementptrInstruction(currentFunc, arrayVar, zeroConst, rowIndex);
            node->blockInsts.addInst(rowGepInst);
            
            // 然后获取列地址：row[0][col]
            GetelementptrInstruction * colGepInst = 
                new GetelementptrInstruction(currentFunc, rowGepInst, zeroConst, colIndex);
            node->blockInsts.addInst(colGepInst);
            
            // 确定要存储的值
            Value * elementValue = nullptr;
            int linearIndex = row * cols + col;
            if (linearIndex < initValues.size()) {
                elementValue = initValues[linearIndex];
            } else {
                elementValue = module->newConstInt(0); // 默认值
            }
            
            // 生成store指令
            StoreInstruction * storeInst = new StoreInstruction(currentFunc, elementValue, colGepInst, 4);
            node->blockInsts.addInst(storeInst);
        }
    }
    
    return true;
}
```

### 5.4 零初始化实现

#### 5.4.1 memset指令生成

```cpp
bool IRGenerator::handleZeroInitialization(ast_node * node, Value * arrayVar, ArrayType * arrayType)
{
    Function * currentFunc = module->getCurrentFunction();
    
    // 1. 将数组转换为 i8*
    BitcastInstruction * destCast = new BitcastInstruction(currentFunc, arrayVar, module->getI8PtrType());
    node->blockInsts.addInst(destCast);
    
    // 2. 计算数组大小
    int totalSize = arrayType->getSize();
    ConstInt * sizeConst = module->newConstInt(totalSize);
    
    // 3. 生成 memset 指令清零
    MemsetInstruction * memsetInst = 
        new MemsetInstruction(currentFunc, destCast, module->newConstInt(0), sizeConst, false);
    node->blockInsts.addInst(memsetInst);
    
    return true;
}
```

#### 5.4.2 生成的IR示例

**C代码：** `int arr[100];` （未初始化）

**生成的IR：**
```llvm
; 分配数组空间
%arr = alloca [100 x i32], align 16

; 转换为i8*进行memset
%1 = bitcast [100 x i32]* %arr to i8*

; 清零整个数组
call void @llvm.memset.p0i8.i32(i8* %1, i8 0, i32 400, i1 false)
```

## 6. 内存管理指令

### 6.1 AllocaInstruction

用于在栈上分配数组空间：

```cpp
class AllocaInstruction : public Instruction {
public:
    AllocaInstruction(Function * _func, Value * _result, Type * _allocatedType, int _align = 4);
    
    void toString(std::string & str) override {
        str = getIRName() + " = alloca " + allocatedType->toString() + ", align " + std::to_string(align);
    }
};
```

### 6.2 MemcpyInstruction

用于数组的批量拷贝：

```cpp
class MemcpyInstruction : public Instruction {
public:
    MemcpyInstruction(Function * _func, Value * _dest, Value * _src, Value * _size, bool _isVolatile);
    
    void toString(std::string & str) override {
        str = "call void @llvm.memcpy.p0i8.p0i8.i32(i8* " + dest->getIRName() + 
              ", i8* " + src->getIRName() + ", i32 " + size->getIRName() + 
              ", i1 " + (isVolatile ? "true" : "false") + ")";
    }
};
```

### 6.3 MemsetInstruction

用于数组的批量设置：

```cpp
class MemsetInstruction : public Instruction {
public:
    MemsetInstruction(Function * _func, Value * _dest, Value * _val, Value * _size, bool _isVolatile);
    
    void toString(std::string & str) override {
        str = "call void @llvm.memset.p0i8.i32(i8* " + dest->getIRName() + 
              ", i8 " + val->getIRName() + ", i32 " + size->getIRName() + 
              ", i1 " + (isVolatile ? "true" : "false") + ")";
    }
};
```

## 7. 复杂数组操作示例

### 7.1 多维数组声明和初始化

**C代码：**
```c
int matrix[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
```

**生成的IR流程：**
1. 创建全局常量数组 `__const.main.matrix`
2. 分配局部数组空间 `%matrix = alloca [3 x [4 x i32]]`
3. 使用memcpy从全局常量数组拷贝到局部数组

### 7.2 数组元素访问和修改

**C代码：**
```c
matrix[1][2] = 100;
int value = matrix[1][2];
```

**生成的IR：**
```llvm
; 计算地址 matrix[1][2]
%1 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %matrix, i32 0, i32 1
%2 = getelementptr inbounds [4 x i32], [4 x i32]* %1, i32 0, i32 2

; 存储值
store i32 100, i32* %2, align 4

; 加载值
%3 = load i32, i32* %2, align 4
```

## 8. 总结

本编译器的IR数组实现具有以下特点：

1. **完整的类型系统**：支持任意维度的数组类型定义
2. **高效的访问机制**：通过GEP指令实现O(1)的数组元素访问
3. **多样的初始化方式**：支持静态、动态和零初始化
4. **内存优化**：使用memcpy和memset进行批量操作
5. **LLVM兼容**：生成的IR完全符合LLVM规范
6. **类型安全**：严格的类型检查和转换
7. **性能优化**：针对不同场景选择最优的实现策略

这个数组实现为编译器提供了强大的数组处理能力，是整个IR生成系统的重要组成部分。
