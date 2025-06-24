# 数组功能实现总结

## 实现概述

成功为编译器前端实现了完整的数组功能，包括一维和多维数组的声明、初始化、访问和LLVM中间代码生成。

## 主要修改

### 1. 扩展类型属性系统 (`AttrType.h`)

**添加维度信息支持:**
```cpp
typedef struct type_attr {
    BasicType type;
    int64_t lineno;
    bool is_array = false;
    std::vector<int> dimensions; // 新增：数组维度信息
} type_attr;
```

### 2. 修复数组类型创建 (`AST.cpp`)

**问题:** 原有的 `typeAttr2Type` 函数不支持数组类型创建。

**解决方案:** 重构函数支持多维数组的正确类型创建：
```cpp
Type * typeAttr2Type(type_attr & attr)
{
    Type * baseType = getBaseType(attr.type);
    
    // 创建嵌套的 ArrayType，从最内层开始构建
    if (attr.is_array && !attr.dimensions.empty()) {
        Type * currentType = baseType;
        // 逆序处理维度：int[2][3] -> [2 x [3 x i32]]
        for (int i = attr.dimensions.size() - 1; i >= 0; i--) {
            std::vector<int> singleDim = {attr.dimensions[i]};
            currentType = new ArrayType(currentType, singleDim);
        }
        return currentType;
    }
    
    return baseType;
}
```

### 3. 完善数组声明处理 (`Antlr4CSTVisitor.cpp`)

**修改 `visitVarDecl` 和 `visitVarDef`:**
- 收集数组维度信息到 `type_attr.dimensions`
- 返回节点和类型信息的配对
- 正确传递维度信息给类型系统

### 4. 增强数组变量声明 (`IRGenerator.cpp`)

**添加数组类型检测:**
```cpp
// 检查变量节点是否有维度信息（数组声明但没有初始化）
if (!varNode->sons.empty()) {
    std::vector<int> dimensions;
    for (auto dimNode : varNode->sons) {
        if (dimNode->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
            dimensions.push_back(dimNode->integer_val);
        }
    }
    
    // 创建数组类型并调用数组处理函数
    ArrayType * arrayType = new ArrayType(typeNode->type, dimensions);
    typeNode->type = arrayType;
    return ir_array_variable_declare_with_init(node, typeNode, varNode, initExprNode);
}
```

### 5. 修复多维数组访问 (`IRGenerator.cpp`)

**问题:** 原有的数组访问逻辑错误地处理嵌套数组访问。

**解决方案:** 重构 `ir_array_access` 函数：
```cpp
// 处理数组基址（支持嵌套访问）
if (arrayNode->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
    // 嵌套数组访问：如 matrix[0][1] 中的 matrix[0] 部分
    if (!ir_visit_ast_node(arrayNode)) {
        return false;
    }
    arrayVar = arrayNode->val; // 这是一个指向子数组的指针
}

// 处理索引表达式（移除错误的嵌套逻辑）
if (!ir_visit_ast_node(indexNode)) {
    return false;
}
// ... 索引处理逻辑
```

### 6. 扩展 GetelementptrInstruction (`GetelementptrInstruction.cpp`)

**添加指针类型支持:**
```cpp
if (baseType->isArrayType()) {
    // 处理数组类型
    const ArrayType * arrayType = static_cast<const ArrayType *>(baseType);
    Type * elementType = arrayType->getElementType();
    this->type = new PointerType(elementType);
} else if (baseType->isPointerType()) {
    // 处理指针类型（多维数组访问的第二层）
    const PointerType * ptrType = static_cast<const PointerType *>(baseType);
    const Type * pointeeType = ptrType->getPointeeType();
    
    if (pointeeType->isArrayType()) {
        const ArrayType * arrayType = static_cast<const ArrayType *>(pointeeType);
        Type * elementType = arrayType->getElementType();
        this->type = new PointerType(elementType);
    }
}
```

## 生成的LLVM IR示例

### 一维数组
```c
int arr[5];
int arr2[3] = {1, 2, 3};
arr[0] = 10;
int value = arr2[1];
```

**生成的IR:**
```llvm
%1 = alloca [5 x i32], align 16                                    ; 数组声明
%2 = alloca [3 x i32], align 16                                    ; 带初始化的数组
@__const.main.arr.0 = constant [3 x i32] [i32 1, i32 2, i32 3]    ; 全局常量数组
call void @llvm.memcpy.p0i8.p0i8.i64(...)                         ; 初始化拷贝

%7 = getelementptr inbounds [5 x i32], [5 x i32]* %1, i64 0, i64 0 ; arr[0]
store i32 10, i32* %7, align 4                                     ; arr[0] = 10

%9 = getelementptr inbounds [3 x i32], [3 x i32]* %2, i64 0, i64 1 ; arr2[1]
%10 = load i32, i32* %9, align 4                                   ; 加载 arr2[1]
```

### 多维数组
```c
int matrix[2][3];
matrix[1][2] = 6;
int value = matrix[1][2];
```

**生成的IR:**
```llvm
%1 = alloca [2 x [3 x i32]], align 16                              ; 二维数组声明

; matrix[1][2] = 6
%14 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 1  ; matrix[1]
%15 = getelementptr inbounds [3 x i32], [3 x i32]* %14, i64 0, i64 2             ; matrix[1][2]
store i32 6, i32* %15, align 4

; int value = matrix[1][2]
%16 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 1  ; matrix[1]
%17 = getelementptr inbounds [3 x i32], [3 x i32]* %16, i64 0, i64 2             ; matrix[1][2]
%18 = load i32, i32* %17, align 4                                                ; 加载值
```

## 测试验证

### 测试用例1: 一维数组
```c
int main() {
    int arr1[5];           // 声明不带初始化
    int arr2[3] = {1, 2, 3}; // 声明并初始化
    arr1[0] = 10;          // 数组赋值
    int value = arr2[1];   // 数组访问
    return value;
}
```

### 测试用例2: 多维数组
```c
int main() {
    int matrix[2][3];      // 二维数组声明
    matrix[0][0] = 1;      // 多维数组赋值
    matrix[1][2] = 6;      
    int value = matrix[1][2]; // 多维数组访问
    return value;          // 应该返回 6
}
```

### 测试用例3: 混合类型数组
```c
float main() {
    float arr[3] = {1.5, 2.5, 3.5}; // 浮点数数组
    float value = arr[1];            // 浮点数数组访问
    return value;                    // 应该返回 2.5
}
```

## 功能特性

✅ **一维数组支持** - 声明、初始化、访问  
✅ **多维数组支持** - 正确的嵌套类型和访问  
✅ **数组初始化** - 使用全局常量数组和 memcpy  
✅ **类型安全** - 正确的类型推导和转换  
✅ **内存管理** - 栈分配和正确的对齐  
✅ **LLVM兼容** - 生成标准的 getelementptr 指令  

## 技术要点

1. **嵌套类型构建**: 多维数组从内到外逐层构建类型
2. **指令生成**: 多维数组访问生成多个 getelementptr 指令
3. **类型推导**: GetelementptrInstruction 正确处理数组和指针类型
4. **内存布局**: 使用标准的 LLVM 数组内存布局
5. **初始化优化**: 使用全局常量数组和 memcpy 进行高效初始化

编译器现在完全支持一维和多维数组的所有基本操作，生成的LLVM IR符合标准且功能完整。
