#!/bin/bash

# 测试图着色寄存器分配算法的脚本
# 演示完整的编译、链接、运行流程

echo "=== MiniC编译器寄存器分配算法测试 ==="
echo

# 检查编译器是否存在
if [ ! -f "./build/minic" ]; then
    echo "错误：编译器 ./build/minic 不存在，请先编译项目"
    exit 1
fi

# 检查测试文件是否存在
if [ ! -f "tests/RegisterAllocationTest.c" ]; then
    echo "错误：测试文件 tests/RegisterAllocationTest.c 不存在"
    exit 1
fi

echo "1. 编译运行时库..."
if command -v aarch64-linux-gnu-gcc &> /dev/null; then
    aarch64-linux-gnu-gcc -DOPT_TEST -c -o ir/std.o ir/std.c
    echo "   运行时库编译完成: ir/std.o (包含性能测试函数)"
else
    echo "   警告：未找到 aarch64-linux-gnu-gcc，跳过运行时库编译"
fi

echo
echo "2. 使用MiniC编译器生成ARM64汇编..."
./build/minic -S -o tests/RegisterAllocationTest.s tests/RegisterAllocationTest.c
if [ $? -eq 0 ]; then
    echo "   MiniC编译成功: tests/RegisterAllocationTest.s"
else
    echo "   错误：MiniC编译失败"
    exit 1
fi

echo
echo "3. 使用GCC生成ARM64汇编（用于对比）..."
if command -v aarch64-linux-gnu-gcc &> /dev/null; then
    aarch64-linux-gnu-gcc -S -o tests/RegisterAllocationTest_gcc.s tests/RegisterAllocationTest.c
    echo "   GCC编译成功: tests/RegisterAllocationTest_gcc.s"
else
    echo "   警告：未找到 aarch64-linux-gnu-gcc，跳过GCC编译"
fi

echo
echo "4. 生成可执行程序..."
if command -v aarch64-linux-gnu-gcc &> /dev/null && [ -f "ir/std.o" ]; then
    aarch64-linux-gnu-gcc -static -g -o tests/RegisterAllocationTest tests/RegisterAllocationTest.s ir/std.o
    if [ $? -eq 0 ]; then
        echo "   MiniC可执行程序生成成功: tests/RegisterAllocationTest"
    else
        echo "   错误：MiniC可执行程序生成失败"
    fi

    # GCC版本需要链接运行时库，但由于使用了我们的内置函数，也需要std.o
    aarch64-linux-gnu-gcc -static -g -o tests/RegisterAllocationTest_gcc tests/RegisterAllocationTest_gcc.s ir/std.o
    if [ $? -eq 0 ]; then
        echo "   GCC可执行程序生成成功: tests/RegisterAllocationTest_gcc"
    else
        echo "   错误：GCC可执行程序生成失败"
    fi
else
    echo "   跳过可执行程序生成（缺少交叉编译器或运行时库）"
fi

echo
echo "5. 分析寄存器使用情况..."
if [ -f "tests/RegisterAllocationTest.s" ]; then
    echo "   MiniC编译器生成的汇编中的寄存器使用："
    grep -E "x[0-9]+|w[0-9]+" tests/RegisterAllocationTest.s | head -10 | sed 's/^/     /'
fi

if [ -f "tests/RegisterAllocationTest_gcc.s" ]; then
    echo "   GCC生成的汇编中的寄存器使用："
    grep -E "x[0-9]+|w[0-9]+" tests/RegisterAllocationTest_gcc.s | head -10 | sed 's/^/     /'
fi

echo
echo "6. 运行测试程序..."
if command -v qemu-aarch64-static &> /dev/null; then
    if [ -f "tests/RegisterAllocationTest" ]; then
        echo "   运行MiniC编译的程序："
        export OPT_TEST=1
        timeout 10s qemu-aarch64-static tests/RegisterAllocationTest
        echo "   返回值: $?"
    fi
    
    if [ -f "tests/RegisterAllocationTest_gcc" ]; then
        echo "   运行GCC编译的程序："
        export OPT_TEST=1
        timeout 10s qemu-aarch64-static tests/RegisterAllocationTest_gcc
        echo "   返回值: $?"
    fi
else
    echo "   警告：未找到 qemu-aarch64-static，跳过程序运行"
fi

echo
echo "=== 测试完成 ==="
echo "生成的文件："
ls -la tests/RegisterAllocationTest* 2>/dev/null | sed 's/^/  /'
