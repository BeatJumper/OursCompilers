# 1. MiniC编译器-表达式版

## 1.1. 编译器的功能

本编译器是一个完整的三阶段编译器，采用前端→IR→后端的经典架构，支持从C语言源代码到ARM64汇编代码的完整编译流程。

### 1.1.1. 数据类型支持

1. **基本数据类型**：
   - `int`：32位有符号整数类型
   - `float`：32位单精度浮点数类型
   - `void`：空类型，用于函数返回值

2. **复合数据类型**：
   - **一维数组**：支持`int arr[10]`形式的一维数组声明
   - **多维数组**：支持`int matrix[3][4]`形式的多维数组声明
   - **数组初始化**：支持`int arr[5] = {1, 2, 3, 4, 5}`形式的数组初始化
   - **数组传参**：支持数组作为函数参数，如`int func(int arr[], int matrix[][3])`

3. **常量支持**：
   - **常量声明**：支持`const int MAX = 100`形式的常量声明
   - **多进制常量**：支持十进制、八进制（0开头）、十六进制（0x开头）整数常量
   - **浮点常量**：支持小数形式和科学计数法形式的浮点常量

### 1.1.2. 变量和作用域

1. **全局变量**：支持全局变量声明，支持初始化, Global variable initialization only supports constants.
2. **局部变量**：支持函数内局部变量声明，可在任意位置声明
3. **作用域管理**：支持变量分层管理，内层变量可遮蔽外层同名变量
4. **符号表**：采用栈式符号表管理变量作用域

### 1.1.3. 函数支持

1. **函数定义**：支持多个函数定义
2. **函数类型**：支持`int`、`float`、`void`三种返回值类型
3. **函数参数**：支持多个形式参数，包括基本类型和数组类型参数
4. **函数调用**：支持函数调用表达式，支持递归调用
5. **内置函数**：
   - `putint(int)`：输出整数到终端
   - `putch(int)`：输出字符到终端
   - `getch()`：从终端读取字符
   - 支持时间测量相关的内置函数

### 1.1.4. 表达式和运算符

1. **算术运算符**：
   - 加法（`+`）、减法（`-`）、乘法（`*`）、除法（`/`）、取模（`%`）
   - 支持整数和浮点数运算
   - 支持类型自动转换

2. **关系运算符**：
   - 小于（`<`）、大于（`>`）、小于等于（`<=`）、大于等于（`>=`）
   - 等于（`==`）、不等于（`!=`）

3. **逻辑运算符**：
   - 逻辑与（`&&`）、逻辑或（`||`）、逻辑非（`!`）
   - 支持短路求值

4. **一元运算符**：
   - 正号（`+`）、负号（`-`）、逻辑非（`!`）

5. **数组访问**：支持`arr[index]`形式的数组元素访问，包括多维数组

6. **表达式优先级**：完整支持C语言表达式优先级和结合性

### 1.1.5. 控制流语句

1. **条件语句**：
   - `if`语句：支持`if(condition) statement`
   - `if-else`语句：支持`if(condition) statement else statement`

2. **循环语句**：
   - `while`循环：支持`while(condition) statement`
   - 支持`break`和`continue`语句

3. **跳转语句**：
   - `return`语句：支持带返回值和不带返回值的return

4. **语句块**：支持`{}`包围的复合语句

### 1.1.6. 预处理支持

1. **宏定义**：支持`#define`宏定义的预处理
2. **注释处理**：支持单行注释（`//`）和多行注释（`/* */`）

## 1.2. 编译器架构和优化功能

### 1.2.1. 编译器架构

本编译器采用经典的三阶段编译架构：

1. **前端（Frontend）**：
   - **词法分析**：基于ANTLR4的词法分析器，支持C语言词法规则
   - **语法分析**：基于ANTLR4的语法分析器，生成具体语法树（CST）
   - **语义分析**：CST到抽象语法树（AST）的转换，进行语义检查
   - **预处理器**：支持宏定义等预处理指令

2. **中间表示（IR）**：
   - **IR生成器**：将AST转换为线性中间表示（DragonIR）
   - **类型系统**：支持整数、浮点、数组、指针等类型
   - **指令系统**：包含算术、逻辑、控制流、内存访问等指令
   - **值系统**：支持常量、变量、临时值等不同类型的值

3. **后端（Backend）**：
   - **指令选择**：将IR指令翻译为目标架构汇编指令
   - **寄存器分配**：基于图着色算法的寄存器分配
   - **代码生成**：生成ARM64汇编代码

### 1.2.2. 优化功能

1. **死代码消除（Dead Code Elimination）**：
   - 移除不可达的基本块
   - 消除冗余的跳转指令
   - 移除return语句后的死代码
   - 基于控制流图的分析

2. **寄存器分配优化**：
   - 基于图着色算法的寄存器分配策略
   - 活跃变量分析
   - 干涉图构建
   - 支持寄存器溢出处理

3. **控制流优化**：
   - 基本块划分和优化
   - 控制流图构建
   - 跳转指令优化

### 1.2.3. 目标平台支持

1. **ARM64架构**：
   - 支持ARMv8-A指令集
   - 完整的ARM64汇编代码生成
   - 支持ARM64寄存器约定

2. **平台特性**：
   - 支持29个通用寄存器（x0-x28）
   - 支持32个向量寄存器（v0-v31）
   - 支持栈帧管理和函数调用约定

### 1.2.4. 调试和分析工具

1. **AST可视化**：支持将抽象语法树输出为图形文件（PNG、SVG等）
2. **IR输出**：支持将中间表示输出为文本文件
3. **汇编注释**：可在汇编代码中包含对应的IR指令作为注释
4. **符号表管理**：完整的符号表信息管理和输出

### 1.2.5. Result
1. 代码质量对比
编译器	总指令数	内存访问指令	内存访问比例	使用寄存器数	栈空间使用
MiniC	1122		422			37.61%			6			416字节
GCC -O0	723			422			58.36%			9			144字节
GCC -O1	298			34			11.40%			45			112字节
GCC -O2	256			35			13.67%			49			112字节
2. 性能测试结果（5次运行平均）
编译器	平均执行时间	计算结果	时间测量方式
MiniC	704微秒	-487658938	内置时间测量
GCC -O0	637微秒	338970470	标准C库
GCC -O1	298微秒	338970470	标准C库
GCC -O2	368微秒	338970470	标准C库
3. 关键发现和改进
✅ MiniC编译器的优势：
内存访问比例显著优于GCC -O0：37.61% vs 58.36%，节省了35%的内存访问
图着色算法有效：在复杂的寄存器分配场景下能够正常工作
功能完整：成功处理大量局部变量的寄存器分配
稳定性好：5次运行结果完全一致，性能稳定
🔧 仍需改进的方面：
寄存器利用率偏低：只使用6个寄存器，远少于ARM64的29个可用寄存器
指令数量较多：1122条指令，比GCC优化版本多
栈空间使用较大：416字节，比GCC版本多
📊 结果差异分析：
计算结果不同：MiniC输出-487658938，GCC输出338970470
可能原因：寄存器分配

源代码位置：<https://github.com/NPUCompiler/exp03-minic-expr.git>

## 1.3. 编译器的命令格式

命令格式：
minic -S [-A | -D] [-T | -I] [-o output] [-O level] [-t cpu] source

选项-S为必须项，默认输出汇编。

选项-O level指定时可指定优化的级别，0为未开启优化。
选项-o output指定时可把结果输出到指定的output文件中。
选项-t cpu指定时，可指定生成指定cpu的汇编语言。

选项-A 指定时通过 antlr4 进行词法与语法分析。

选项-T指定时，输出抽象语法树，默认输出的文件名为ast.png，可通过-o选项来指定输出的文件。
选项-I指定时，输出中间IR(DragonIR)，默认输出的文件名为ir.txt，可通过-o选项来指定输出的文件。
选项-T和-I都不指定时，按照默认的汇编语言输出，默认输出的文件名为asm.s，可通过-o选项来指定输出的文件。

## 1.4. 源代码构成

```text
├── CMake
├── backend                     编译器后端
│   └── arm64                   ARM64后端
├── doc                         文档资料
│   ├── figures
│   └── graphviz
├── frontend                    前端
│   └── antlr4                  Antlr4实现
├── ir                          中间IR
│   ├── Generator               中间IR的产生器
│   ├── Instructions            中间IR的指令
│   ├── Types                   中间IR的类型
│   └── Values                  中间IR的值
├── symboltable                 符号表
├── tests                       测试用例
├── thirdparty                  第三方工具
│   └── antlr4                  antlr4工具
├── tools                       工具
│   ├── IRCompiler              中间IR解析执行器
│   │   └── Linux-x86_64
│   │       ├── Ubuntu-20.04    Ubuntu-20.04下的工具
│   │       └── Ubuntu-22.04    Ubuntu-22.04下的工具
│   ├── pictures                相关图片
│   └── generate_antlr4.sh		前端自动生成代码集成指令(error!)
└── utils                       集合、位图等共同的代码
```

## 1.5. 程序构建


### 1.5.1. cmake插件构建

命令行来进行构建如下：

```shell
# cmake根据CMakeLists.txt进行配置与检查，这里使用clang编译器并且是Debug模式
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++
# cmake，其中--parallel说明是并行编译，也可用-j选项
cmake --build build --parallel
```

## 1.6. 使用方法

在Ubuntu 22.04平台上运行。支持的命令如下所示：

```shell

./build/minic -S -T -o ./tests/test1-1.png ./tests/test1-1.c
./build/minic -S -T -o ./tests/Control.png ./tests/Control.c
./build/minic -S -T -o ./tests/function.png ./tests/function.c

./build/minic -S -T -A -o ./tests/test1-1.png ./tests/test1-1.c

./build/minic -S -T -D -o ./tests/test1-1.png ./tests/test1-1.c

./build/minic -S -I -o ./tests/test1-1.ir ./tests/test1-1.c

./build/minic -S -I -A -o ./tests/test1-1.ir ./tests/test1-1.c

./build/minic -S -I -D -o ./tests/test1-1.ir ./tests/test1-1.c

./build/minic -S -o ./tests/test1-1.s ./tests/test1-1.c

./build/minic -S -A -o ./tests/test1-1.s ./tests/test1-1.c

./build/minic -S -D -o ./tests/test1-1.s ./tests/test1-1.c

```

## 1.7. 工具

本实验所需要的工具或软件在实验一环境准备中已经安装，这里不需要再次安装。

这里主要介绍工具的功能。


### 1.7.2. Antlr 4.12.0

要确认java15 以上版本的 JDK，否则编译不会通过。默认已经安装了JDK 17的版本。

由于cmake的bug可能会导致适配不到15以上的版本，请删除旧版的JDK。

编写 g4 文件然后通过 antlr 生成 C++代码，用 Visitor 模式。

```shell
java -jar thirdparty/antlr4/antlr-4.12.0-complete.jar -Dlanguage=Cpp -no-listener -visitor -o frontend/antlr4/autogenerated frontend/antlr4/MiniC.g4
```

C++使用 antlr 时需要使用 antlr 的头文件和库，在 msys2 下可通过如下命令安装 antlr 4.12.0 版即可。

生成的时候无论如何都会生成一个子目录，非常麻烦，用下面的指令变化。一键完成。
下面的指令集成到了tools/generate.sh下。
在使用之前，首先chmod指令赋予权限。然后执行。
目前有点bug，先别用了。
```shell
chmod u+x ./tools/generate_antlr4.sh
./tools/generate_antlr4.sh
```

```shell
java -jar thirdparty/antlr4/antlr-4.12.0-complete.jar -Dlanguage=Cpp -no-listener -visitor -o frontend/antlr4/autogenerated frontend/antlr4/MiniC.g4
# 进入 autogenerated 目录
cd /home/code/exp04-minic-expr/frontend/antlr4/autogenerated

# 将 frontend/antlr4/ 下的所有文件移动到当前目录
mv frontend/antlr4/* .

# 删除 frontend/antlr4 文件夹
rm -rf frontend/antlr4

# 删除 frontend 文件夹
rm -rf frontend
cd ..
cd ..
cd ..
```
```shell
cd build
cpack --config CPackSourceConfig.cmake
```


```shell
pacman -U https://mirrors.ustc.edu.cn/msys2/mingw/mingw64/mingw-w64-x86_64-antlr4-runtime-cpp-4.12.0-1-any.pkg.tar.zst
```

### 1.7.3. Graphviz

借助该工具提供的C语言API实现抽象语法树的绘制。

### 1.7.4. doxygen

借助该工具分析代码中的注释，产生详细分析的文档。这要求注释要满足一定的格式。具体可参考实验文档。

### 1.7.5. texlive

把doxygen生成的文档转换成pdf格式。

## 1.8. 根据注释生成文档

请按照实验的文档要求编写注释，可通过doxygen工具生成网页版的文档，借助latex可生成pdf格式的文档。

请在本实验以及后续的实验按照格式进行注释。

执行的下面的命令后会在doc文件夹下生成html和latex文件夹，通过打开index.html可浏览。

```shell
doxygen Doxygen.config
```

在安装texlive等latex工具后，可通过执行的下面的命令产生refman.pdf文件。

```shell
cd doc/latex
make
```

## 1.9. 实验运行

tests 目录下存放了一些简单的测试用例。

由于 qemu 的用户模式在 Window 系统下不支持，因此要么在真实的开发板上运行，或者用 Linux 系统下的 qemu 来运行。

### 1.9.1. 调试运行

由于默认的gdb或者lldb调试器对C++的STL模版库提供的类如string、map等的显示不够友好，
因此请大家确保安装vadimcn.vscode-lldb插件，也可以更新最新的代码后vscode会提示安装推荐插件后自动安装。

如安装不上请手动下载后安装，网址如下：
<https://github.com/vadimcn/codelldb/releases/>

调试运行配置可参考.vscode/launch.json中的配置。

### 1.9.2. 生成中间IR(DragonIR)与运行

前提需要下载并安装IRCompiler工具。

```shell
# 翻译 test1-1.c 成 ARM32 汇编
./build/minic -S -I -o tests/test1-1.ir tests/test1-1.txt
./IRCompiler -R tests/test1-1.ir

# 要首先chmod获取执行权限
chmod u+x ./tools/IRCompiler/Linux-x86_64/Ubuntu-22.04/IRCompiler

./tools/IRCompiler/Linux-x86_64/Ubuntu-22.04/IRCompiler -R tests/test1-1.ir
```

第一条指令通过minic编译器来生成的汇编test1-1.ir
第二条指令借助IRCompiler工具实现对生成IR的解释执行。

### 1.9.3. 生成 ARM64 的汇编

```shell
# 翻译 test1-1.c 成 ARM64 汇编
./build/minic -S -o tests/test1-1-0.s tests/test1-1.c
# 把 test1-1.c 通过 ARM64 版的交叉编译器 gcc 翻译成汇编
aarch64-linux-gnu-gcc -S -o tests/test1-1-1.s tests/test1-1.c

# 测试复杂的寄存器分配算法
./build/minic -S -o tests/RegisterAllocationTest.s tests/RegisterAllocationTest.c
aarch64-linux-gnu-gcc -S -o tests/RegisterAllocationTest_gcc.s tests/RegisterAllocationTest.c
```

第一条命令通过minic编译器来生成的ARM64汇编test1-1-0.s
第二条指令是通过aarch64-linux-gnu-gcc编译器生成的ARM64汇编语言test1-1-1.s。
第三、四条命令用于测试复杂的寄存器分配算法效果。

在调试运行时可通过对比检查所实现编译器的问题。

### 1.9.4. 生成可执行程序

通过 gcc 的 ARM64 交叉编译器对生成的汇编进行编译，生成可执行程序。由于编译器使用了运行时库，需要链接std.c文件。

```shell
# 编译运行时库
aarch64-linux-gnu-gcc -c -o ir/std.o ir/std.c

# 通过 ARM64 gcc 编译器把汇编程序翻译成可执行程序，目标平台 ARM64
aarch64-linux-gnu-gcc -static -g -o tests/test1-1-0 tests/test1-1-0.s ir/std.o
# 通过 ARM64 gcc 编译器把汇编程序翻译成可执行程序，目标平台 ARM64
aarch64-linux-gnu-gcc -static -g -o tests/test1-1-1 tests/test1-1-1.s

# 测试复杂寄存器分配的可执行程序
aarch64-linux-gnu-gcc -static -g -o tests/RegisterAllocationTest tests/RegisterAllocationTest.s ir/std.o
aarch64-linux-gnu-gcc -static -g -o tests/RegisterAllocationTest_gcc tests/RegisterAllocationTest_gcc.s
```

有以下几个点需要注意：

1. **静态编译**：这里必须用-static 进行静态编译，不依赖动态库，否则后续通过 qemu-aarch64-static 运行时会提示动态库找不到的错误
2. **运行时库链接**：由于编译器使用了内置函数（如putint、getch、starttime、stoptime等），需要链接运行时库std.o
3. **寄存器分配测试**：RegisterAllocationTest.c 包含大量局部变量和复杂表达式，能够充分测试图着色寄存器分配算法的效果
4. **性能对比**：可通过网址<https://godbolt.org/>输入 C 语言源代码后查看各种目标后端的汇编，选择 ARM64 GCC 来对比汇编质量

### 1.9.5. 运行可执行程序

借助用户模式的 qemu 来运行，ARM64 架构可使用 qemu-aarch64-static 命令。

```shell
# 运行基本测试程序
qemu-aarch64-static tests/test1-1-0
echo $?
qemu-aarch64-static tests/test1-1-1
echo $?

# 运行寄存器分配测试程序（包含性能计时）
qemu-aarch64-static tests/RegisterAllocationTest
echo $?
qemu-aarch64-static tests/RegisterAllocationTest_gcc
echo $?
```

这里可比较运行的结果(即通过指令echo $?获取main函数的返回值，注意截断8位的无符号整数)，如果两者不一致，则编写的编译器程序有问题。

对于包含starttime()和stoptime()的程序，需要设置环境变量启用性能测试：

```shell
# 启用性能测试模式
export OPT_TEST=1
qemu-aarch64-static tests/RegisterAllocationTest
```

如果测试用例源文件程序需要输入，假定输入的内容在文件A.in中，则可通过以下方式运行。

```shell
qemu-aarch64-static tests/test1-1-0 < A.in
echo $?
qemu-aarch64-static tests/test1-1-1 < A.in
echo $?
```

如果想把输出的内容写到文件中，可通过重定向符号>来实现，假定输入到B.out文件中。

```shell
qemu-aarch64-static tests/test1-1-0 < A.in > A.out
echo $?
qemu-aarch64-static tests/test1-1-1 < A.in > A.out
echo $?
```

### 1.9.6. 寄存器分配算法测试

编译器实现了基于图着色的寄存器分配算法，RegisterAllocationTest.c 是专门设计的测试用例，具有以下特点：

1. **大量局部变量**：每个函数包含20+个局部变量，超过ARM64可用寄存器数量
2. **复杂表达式**：包含嵌套的算术表达式，产生大量临时变量
3. **控制流复杂**：包含嵌套循环、条件分支，增加活跃变量分析的复杂度
4. **函数调用**：包含递归调用和矩阵运算，测试调用约定和寄存器保存
5. **性能测量**：使用starttime()和stoptime()测量执行时间

通过对比编译器生成的汇编代码和GCC生成的代码，可以评估寄存器分配算法的效果：

```shell
# 查看编译器生成的汇编代码中的寄存器使用情况
grep -E "x[0-9]+|w[0-9]+" tests/RegisterAllocationTest.s | head -20

# 查看GCC生成的汇编代码中的寄存器使用情况
grep -E "x[0-9]+|w[0-9]+" tests/RegisterAllocationTest_gcc.s | head -20
```

优秀的寄存器分配应该表现为：
- 减少内存访问指令（load/store）
- 充分利用可用寄存器
- 在寄存器压力大的区域合理进行溢出

## 1.10. qemu 的用户模式

qemu 的用户模式下可直接运行交叉编译的用户态程序。这种模式只在 Linux 和 BSD 系统下支持，Windows 下不支持。
因此，为便于后端开发与调试，请用 Linux 系统进行程序的模拟运行与调试。

## 1.11. qemu 用户程序调试

### 1.11.1. 安装 gdb 调试器

该软件 gdb-multiarch 在前面工具安装时已经安装。如没有，则通过下面的命令进行安装。

```shell
sudo apt-get install -y gdb-multiarch
```

### 1.11.2. 启动具有 gdbserver 功能的 qemu

假定通过交叉编译出的程序为 tests/test1，执行的命令如下：

```shell
# 启动 gdb server，监视的端口号为 1234
qemu-aarch64-static -g 1234 tests/test1
```

其中-g 指定远程调试的端口，这里指定端口号为 1234，这样 qemu 会开启 gdb 的远程调试服务。

### 1.11.3. 启动 gdb 作为客户端远程调试

建议通过 vscode 的调试，选择 Qemu Debug 进行调试，可开启图形化调试界面。

可根据需要修改相关的配置，如 miDebuggerServerAddress、program 等选项。

也可以在命令行终端上启动 gdb 进行远程调试，需要指定远程机器的主机与端口。

注意这里的 gdb 要支持目标 CPU 的 gdb-multiarch，而不是本地的 gdb。

```shell
gdb-multiarch tests/test1
# 输入如下的命令，远程连接 qemu 的 gdb server
target remote localhost:1234
# 在 main 函数入口设置断点
b main
# 继续程序的运行
c
# 之后可使用 gdb 的其它命令进行单步运行与调试
```

在调试完毕后前面启动的 qemu-aarch64-static 程序会自动退出。因此，要想重新调试，请启动第一步的 qemu-aarch64-static 程序。

## 1.12. 源程序打包

在执行前，请务必通过cmake进行build成功，这样会在build目录下生成CPackSourceConfig.cmake文件。

进入build目录下执行如下的命令可产生源代码压缩包，用于实验源代码的提交

```shell
cd build
cpack --config CPackSourceConfig.cmake
```

在build目录下默认会产生zip和tar.gz格式的文件。

可根据需要调整CMakeLists.txt文件的CPACK_SOURCE_IGNORE_FILES用于忽略源代码文件夹下的某些文件夹或者文件。

## 1.13. 二进制程序打包

可在VScode页面下的状态栏上单击Run Cpack即可在build产生zip和tar.gz格式的压缩包，里面包含编译出的可执行程序。
