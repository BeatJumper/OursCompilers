# 前端预处理器实现详解

## 1. 概述

本编译器的前端预处理器是一个简单而高效的预处理模块，主要负责处理C语言源代码中的预处理指令。预处理器在词法分析之前运行，对源代码进行预处理转换，为后续的词法和语法分析做准备。

## 2. 架构设计

### 2.1 核心组件

预处理器由以下核心文件组成：

- **`frontend/antlr4/Preprocessor.h`** - 预处理器类的头文件定义
- **`frontend/antlr4/Preprocessor.cpp`** - 预处理器类的具体实现
- **`frontend/antlr4/Antlr4Executor.cpp`** - 集成预处理器到前端流程

### 2.2 类设计

```cpp
class Preprocessor {
public:
    Preprocessor() = default;
    ~Preprocessor() = default;
    
    std::string process(const std::string & sourceCode);

private:
    std::unordered_map<std::string, std::string> macroDefinitions;
    
    bool parseDefine(const std::string & line);
    std::string replaceMacros(const std::string & text);
    std::string processTimingMacros(const std::string & line, int lineNumber);
    bool isIdentifierChar(char c);
};
```

## 3. 主要功能

### 3.1 宏定义处理

#### 3.1.1 宏定义解析

预处理器支持标准的`#define`宏定义语法：

```c
#define MACRO_NAME MACRO_VALUE
```

**实现机制：**

1. **正则表达式匹配**：使用正则表达式`^\s*#define\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+)\s*$`来解析宏定义
2. **宏名验证**：宏名必须符合C语言标识符规则（字母或下划线开头，后跟字母、数字或下划线）
3. **宏值处理**：自动去除宏值末尾的空白字符
4. **存储管理**：使用`std::unordered_map`存储宏定义，键为宏名，值为宏值

#### 3.1.2 宏替换算法

宏替换采用字符串搜索和替换的方式：

```cpp
std::string Preprocessor::replaceMacros(const std::string & text)
{
    std::string result = text;
    
    for (const auto & macro: macroDefinitions) {
        const std::string & macroName = macro.first;
        const std::string & macroValue = macro.second;
        
        size_t pos = 0;
        while ((pos = result.find(macroName, pos)) != std::string::npos) {
            // 检查前后字符，确保是完整的标识符
            bool validStart = (pos == 0) || !isIdentifierChar(result[pos - 1]);
            bool validEnd = (pos + macroName.length() >= result.length()) || 
                           !isIdentifierChar(result[pos + macroName.length()]);
            
            if (validStart && validEnd) {
                result.replace(pos, macroName.length(), macroValue);
                pos += macroValue.length();
            } else {
                pos += macroName.length();
            }
        }
    }
    
    return result;
}
```

**关键特性：**
- **边界检查**：确保只替换完整的标识符，避免部分匹配
- **字符验证**：使用`isIdentifierChar()`函数检查字符是否为标识符字符
- **迭代替换**：支持多个宏定义的连续替换

### 3.2 时间函数宏处理

#### 3.2.1 特殊宏函数

预处理器特别支持两个时间测量函数的宏替换：

- `starttime()` → `_sysy_starttime(line_number)`
- `stoptime()` → `_sysy_stoptime(line_number)`

#### 3.2.2 实现机制

```cpp
std::string Preprocessor::processTimingMacros(const std::string & line, int lineNumber)
{
    std::string result = line;
    
    // 处理starttime()调用
    std::regex starttimeRegex(R"(\bstarttime\s*\(\s*\))");
    std::string starttimeReplacement = "_sysy_starttime(" + std::to_string(lineNumber) + ")";
    result = std::regex_replace(result, starttimeRegex, starttimeReplacement);
    
    // 处理stoptime()调用
    std::regex stoptimeRegex(R"(\bstoptime\s*\(\s*\))");
    std::string stoptimeReplacement = "_sysy_stoptime(" + std::to_string(lineNumber) + ")";
    result = std::regex_replace(result, stoptimeRegex, stoptimeReplacement);
    
    return result;
}
```

**特点：**
- **行号注入**：自动将当前行号作为参数传递给运行时函数
- **正则匹配**：使用正则表达式精确匹配函数调用模式
- **运行时支持**：转换后的函数需要运行时库支持（std.c/std.h）

## 4. 处理流程

### 4.1 两遍扫描算法

预处理器采用两遍扫描的处理策略：

```cpp
std::string Preprocessor::process(const std::string & sourceCode)
{
    std::istringstream iss(sourceCode);
    std::ostringstream oss;
    std::string line;
    int lineNumber = 1;
    
    // 第一遍：解析所有的#define指令，同时处理starttime()和stoptime()
    while (std::getline(iss, line)) {
        if (line.find("#define") == 0) {
            if (parseDefine(line)) {
                lineNumber++;
                continue; // 跳过#define行
            }
        }
        
        // 处理时间宏
        line = processTimingMacros(line, lineNumber);
        oss << line << "\n";
        lineNumber++;
    }
    
    // 第二遍：替换所有的宏
    std::string processedCode = oss.str();
    return replaceMacros(processedCode);
}
```

**第一遍扫描：**
1. 解析并收集所有`#define`宏定义
2. 处理`starttime()`和`stoptime()`特殊宏
3. 移除`#define`行，保留其他代码

**第二遍扫描：**
1. 对整个代码进行宏替换
2. 返回处理后的源代码

### 4.2 集成到前端流程

预处理器在前端执行流程中的位置：

```cpp
// Antlr4Executor::run()中的集成
bool Antlr4Executor::run()
{
    // 1. 读取源文件
    std::ifstream ifs(filename);
    std::ostringstream oss;
    oss << ifs.rdbuf();
    std::string sourceCode = oss.str();
    
    // 2. 预处理：处理#define宏定义
    Preprocessor preprocessor;
    std::string processedCode = preprocessor.process(sourceCode);
    
    // 3. 词法分析
    antlr4::ANTLRInputStream input{processedCode};
    MiniCLexer lexer{&input};
    
    // 4. 语法分析
    antlr4::CommonTokenStream tokenStream{&lexer};
    MiniCParser parser{&tokenStream};
    
    // 5. 生成CST
    auto cstRoot = parser.compileUnit();
    
    // 6. CST转AST
    MiniCCSTVisitor visitor;
    astRoot = visitor.run(cstRoot);
    
    return true;
}
```

## 5. 与ANTLR4的协作

### 5.1 词法规则配置

在ANTLR4语法文件中，预处理指令被配置为跳过处理：

```antlr
// 预处理指令：#define 宏定义，跳过处理（由预处理器处理）
PREPROCESSOR_DEFINE: '#define' ~[\r\n]* -> skip;
```

这确保了：
- 预处理器处理完成后，ANTLR4不会再次处理`#define`指令
- 避免了词法分析阶段的冲突
- 保持了前端流程的清晰分离

### 5.2 注释处理

预处理器与ANTLR4协作处理注释：

```antlr
// 单行注释：从 // 开始到行末，跳过处理
LINE_COMMENT: '//' ~[\r\n]* -> skip;

// 多行注释：从 /* 开始到 */ 结束，可跨行，跳过处理
BLOCK_COMMENT: '/*' .*? '*/' -> skip;
```

## 6. 设计优势

### 6.1 模块化设计
- **职责单一**：预处理器专注于预处理功能
- **接口清晰**：通过简单的`process()`方法提供服务
- **易于扩展**：可以轻松添加新的预处理功能

### 6.2 性能优化
- **两遍扫描**：避免了复杂的递归替换
- **哈希表存储**：宏定义查找效率高
- **字符串操作优化**：减少不必要的字符串拷贝

### 6.3 错误处理
- **正则表达式验证**：确保宏定义语法正确
- **边界检查**：避免错误的宏替换
- **行号跟踪**：为时间函数提供准确的行号信息

## 7. 使用示例

### 7.1 基本宏定义

**输入代码：**
```c
#define MAX_SIZE 100
#define PI 3.14159

int array[MAX_SIZE];
float radius = PI * 2;
```

**预处理后：**
```c
int array[100];
float radius = 3.14159 * 2;
```

### 7.2 时间函数处理

**输入代码：**
```c
int main() {
    starttime();
    // some computation
    stoptime();
    return 0;
}
```

**预处理后：**
```c
int main() {
    _sysy_starttime(2);
    // some computation
    _sysy_stoptime(4);
    return 0;
}
```

## 8. 总结

本编译器的前端预处理器实现了一个简洁而功能完整的预处理系统，主要特点包括：

1. **功能完整**：支持标准的`#define`宏定义和特殊的时间函数宏
2. **设计清晰**：采用两遍扫描算法，逻辑清晰易懂
3. **集成良好**：与ANTLR4前端无缝集成
4. **性能良好**：使用高效的数据结构和算法
5. **易于维护**：代码结构清晰，注释完整

这个预处理器为整个编译器前端提供了坚实的基础，确保了源代码在进入词法和语法分析阶段之前得到正确的预处理。
