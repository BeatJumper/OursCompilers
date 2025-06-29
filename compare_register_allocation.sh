#!/bin/bash

# 寄存器分配对比测试脚本
# 对比MiniC编译器和GCC的寄存器分配效果

echo "=== 寄存器分配算法对比测试 ==="
echo

# 检查编译器是否存在
if [ ! -f "./build/minic" ]; then
    echo "错误：编译器 ./build/minic 不存在，请先编译项目"
    exit 1
fi

# 检查测试文件是否存在
if [ ! -f "tests/RegisterAllocationCompare_Pure.c" ]; then
    echo "错误：测试文件 tests/RegisterAllocationCompare_Pure.c 不存在"
    exit 1
fi

echo "1. 编译运行时库..."
if command -v aarch64-linux-gnu-gcc &> /dev/null; then
    aarch64-linux-gnu-gcc -DOPT_TEST -c -o ir/std.o ir/std.c
    echo "   运行时库编译完成"
else
    echo "   警告：未找到 aarch64-linux-gnu-gcc，跳过运行时库编译"
fi

echo
echo "2. 使用MiniC编译器生成汇编..."
./build/minic -S -o tests/RegisterAllocationCompare_minic.s tests/RegisterAllocationCompare_Pure.c
if [ $? -eq 0 ]; then
    echo "   MiniC编译成功"
else
    echo "   错误：MiniC编译失败"
    exit 1
fi

echo
echo "3. 使用GCC生成汇编..."
if command -v aarch64-linux-gnu-gcc &> /dev/null; then
    aarch64-linux-gnu-gcc -S -O0 -o tests/RegisterAllocationCompare_gcc_O0.s tests/RegisterAllocationCompare_Pure.c
    aarch64-linux-gnu-gcc -S -O1 -o tests/RegisterAllocationCompare_gcc_O1.s tests/RegisterAllocationCompare_Pure.c
    aarch64-linux-gnu-gcc -S -O2 -o tests/RegisterAllocationCompare_gcc_O2.s tests/RegisterAllocationCompare_Pure.c
    echo "   GCC编译成功（O0, O1, O2优化级别）"
else
    echo "   警告：未找到 aarch64-linux-gnu-gcc，跳过GCC编译"
fi

echo
echo "4. 生成可执行程序..."
if command -v aarch64-linux-gnu-gcc &> /dev/null; then
    # MiniC版本（需要链接运行时库）
    aarch64-linux-gnu-gcc -static -g -o tests/RegisterAllocationCompare_minic tests/RegisterAllocationCompare_minic.s ir/std.o
    if [ $? -eq 0 ]; then
        echo "   MiniC可执行程序生成成功"
    else
        echo "   错误：MiniC可执行程序生成失败"
    fi
    
    # GCC版本（不同优化级别）- 由于使用了我们的内置函数，也需要链接std.o
    aarch64-linux-gnu-gcc -static -g -O0 -o tests/RegisterAllocationCompare_gcc_O0 tests/RegisterAllocationCompare_gcc_O0.s ir/std.o
    aarch64-linux-gnu-gcc -static -g -O1 -o tests/RegisterAllocationCompare_gcc_O1 tests/RegisterAllocationCompare_gcc_O1.s ir/std.o
    aarch64-linux-gnu-gcc -static -g -O2 -o tests/RegisterAllocationCompare_gcc_O2 tests/RegisterAllocationCompare_gcc_O2.s ir/std.o
    echo "   GCC可执行程序生成成功（O0, O1, O2）"
else
    echo "   跳过可执行程序生成"
fi

echo
echo "5. 分析汇编代码质量..."

analyze_assembly() {
    local file=$1
    local name=$2
    
    if [ -f "$file" ]; then
        echo "   === $name ==="
        
        # 统计指令数量
        local total_insts=$(grep -E "^\s*[a-z]" "$file" | wc -l)
        echo "     总指令数: $total_insts"
        
        # 统计load/store指令（内存访问）
        local memory_insts=$(grep -E "^\s*(ldr|str|ldp|stp)" "$file" | wc -l)
        echo "     内存访问指令: $memory_insts"
        
        # 统计寄存器使用情况
        local reg_usage=$(grep -oE "x[0-9]+|w[0-9]+" "$file" | sort | uniq | wc -l)
        echo "     使用的寄存器数量: $reg_usage"
        
        # 计算内存访问比例
        if [ $total_insts -gt 0 ]; then
            local memory_ratio=$(echo "scale=2; $memory_insts * 100 / $total_insts" | bc -l 2>/dev/null || echo "N/A")
            echo "     内存访问比例: ${memory_ratio}%"
        fi
        
        # 显示函数开头的寄存器使用模式
        echo "     寄存器使用模式示例:"
        grep -E "^\s*(mov|add|sub|mul|ldr|str)" "$file" | head -8 | sed 's/^/       /'
        echo
    fi
}

analyze_assembly "tests/RegisterAllocationCompare_minic.s" "MiniC编译器"
analyze_assembly "tests/RegisterAllocationCompare_gcc_O0.s" "GCC -O0"
analyze_assembly "tests/RegisterAllocationCompare_gcc_O1.s" "GCC -O1"
analyze_assembly "tests/RegisterAllocationCompare_gcc_O2.s" "GCC -O2"

echo
echo "6. 运行性能测试..."

run_performance_test() {
    local executable=$1
    local name=$2
    
    if [ -f "$executable" ] && command -v qemu-aarch64-static &> /dev/null; then
        echo "   === $name ==="
        echo "     运行结果:"
        timeout 30s qemu-aarch64-static "$executable" | sed 's/^/       /'
        echo "     返回值: $?"
        echo
    fi
}

run_performance_test "tests/RegisterAllocationCompare_minic" "MiniC编译器"
run_performance_test "tests/RegisterAllocationCompare_gcc_O0" "GCC -O0"
run_performance_test "tests/RegisterAllocationCompare_gcc_O1" "GCC -O1"
run_performance_test "tests/RegisterAllocationCompare_gcc_O2" "GCC -O2"

echo
echo "7. 详细寄存器分配分析..."

detailed_register_analysis() {
    local file=$1
    local name=$2
    
    if [ -f "$file" ]; then
        echo "   === $name 详细分析 ==="
        
        # 分析heavy_computation函数的寄存器分配
        echo "     heavy_computation函数寄存器分配:"
        awk '/heavy_computation:/{flag=1} flag && /^[a-zA-Z_][a-zA-Z0-9_]*:/{if(!/heavy_computation:/)flag=0} flag' "$file" | \
        grep -E "^\s*(mov|add|sub|mul|ldr|str)" | head -15 | sed 's/^/       /'
        
        echo
        
        # 统计栈空间使用
        local stack_usage=$(grep -oE "sp.*#[0-9]+" "$file" | grep -oE "#[0-9]+" | sort -n | tail -1 | tr -d '#')
        echo "     栈空间使用: ${stack_usage:-0} 字节"
        
        echo
    fi
}

detailed_register_analysis "tests/RegisterAllocationCompare_minic.s" "MiniC编译器"
detailed_register_analysis "tests/RegisterAllocationCompare_gcc_O0.s" "GCC -O0"
detailed_register_analysis "tests/RegisterAllocationCompare_gcc_O1.s" "GCC -O1"

echo
echo "=== 对比总结 ==="
echo "1. 指令数量对比：数量越少表示代码越紧凑"
echo "2. 内存访问对比：比例越低表示寄存器利用越好"
echo "3. 寄存器使用对比：合理利用可用寄存器"
echo "4. 性能对比：执行时间越短表示优化越好"
echo
echo "生成的文件："
ls -la tests/RegisterAllocationCompare* 2>/dev/null | sed 's/^/  /'
