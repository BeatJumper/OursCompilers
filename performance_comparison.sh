#!/bin/bash

# 性能对比测试脚本
# 分别使用MiniC和GCC编译相同算法的程序，使用各自的时间测量方法

echo "=== MiniC vs GCC 寄存器分配性能对比测试 ==="
echo

# 检查编译器是否存在
if [ ! -f "./build/minic" ]; then
    echo "错误：编译器 ./build/minic 不存在，请先编译项目"
    exit 1
fi

# 检查测试文件是否存在
if [ ! -f "tests/RegisterTest_MiniC.c" ] || [ ! -f "tests/RegisterTest_GCC.c" ]; then
    echo "错误：测试文件不存在"
    exit 1
fi

echo "1. 编译运行时库..."
if command -v aarch64-linux-gnu-gcc &> /dev/null; then
    aarch64-linux-gnu-gcc -DOPT_TEST -c -o ir/std.o ir/std.c
    echo "   运行时库编译完成"
else
    echo "   警告：未找到 aarch64-linux-gnu-gcc，跳过运行时库编译"
    exit 1
fi

echo
echo "2. 编译MiniC版本..."
./build/minic -S -o tests/RegisterTest_MiniC.s tests/RegisterTest_MiniC.c
if [ $? -eq 0 ]; then
    echo "   MiniC汇编生成成功"
    aarch64-linux-gnu-gcc -static -g -o tests/RegisterTest_MiniC tests/RegisterTest_MiniC.s ir/std.o
    if [ $? -eq 0 ]; then
        echo "   MiniC可执行程序生成成功"
    else
        echo "   错误：MiniC可执行程序生成失败"
        exit 1
    fi
else
    echo "   错误：MiniC编译失败"
    exit 1
fi

echo
echo "3. 编译GCC版本（不同优化级别）..."
aarch64-linux-gnu-gcc -S -O0 -o tests/RegisterTest_GCC_O0.s tests/RegisterTest_GCC.c
aarch64-linux-gnu-gcc -S -O1 -o tests/RegisterTest_GCC_O1.s tests/RegisterTest_GCC.c
aarch64-linux-gnu-gcc -S -O2 -o tests/RegisterTest_GCC_O2.s tests/RegisterTest_GCC.c

aarch64-linux-gnu-gcc -static -g -O0 -o tests/RegisterTest_GCC_O0 tests/RegisterTest_GCC.c
aarch64-linux-gnu-gcc -static -g -O1 -o tests/RegisterTest_GCC_O1 tests/RegisterTest_GCC.c
aarch64-linux-gnu-gcc -static -g -O2 -o tests/RegisterTest_GCC_O2 tests/RegisterTest_GCC.c
echo "   GCC编译成功（O0, O1, O2）"

echo
echo "4. 汇编代码质量分析..."

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
        echo
    fi
}

analyze_assembly "tests/RegisterTest_MiniC.s" "MiniC编译器"
analyze_assembly "tests/RegisterTest_GCC_O0.s" "GCC -O0"
analyze_assembly "tests/RegisterTest_GCC_O1.s" "GCC -O1"
analyze_assembly "tests/RegisterTest_GCC_O2.s" "GCC -O2"

echo
echo "5. 性能测试（多次运行取平均值）..."

run_performance_test() {
    local executable=$1
    local name=$2
    local runs=5
    
    if [ -f "$executable" ] && command -v qemu-aarch64-static &> /dev/null; then
        echo "   === $name ==="
        
        local total_time=0
        local successful_runs=0
        
        for i in $(seq 1 $runs); do
            echo "     第 $i 次运行:"
            local output=$(timeout 30s qemu-aarch64-static "$executable" 2>&1)
            local exit_code=$?
            
            if [ $exit_code -eq 0 ]; then
                echo "$output" | sed 's/^/       /'
                successful_runs=$((successful_runs + 1))
                
                # 尝试提取时间信息（如果有的话）
                local time_info=$(echo "$output" | grep -E "(microseconds|us)" | head -1)
                if [ ! -z "$time_info" ]; then
                    echo "       时间: $time_info"
                fi
            else
                echo "       运行失败 (退出码: $exit_code)"
            fi
            echo
        done
        
        echo "     成功运行: $successful_runs/$runs 次"
        echo
    else
        echo "   跳过 $name（文件不存在或缺少qemu）"
    fi
}

# 设置性能测试环境变量
export OPT_TEST=1

run_performance_test "tests/RegisterTest_MiniC" "MiniC编译器"
run_performance_test "tests/RegisterTest_GCC_O0" "GCC -O0"
run_performance_test "tests/RegisterTest_GCC_O1" "GCC -O1"
run_performance_test "tests/RegisterTest_GCC_O2" "GCC -O2"

echo
echo "6. 详细寄存器分配对比..."

detailed_comparison() {
    echo "   === 寄存器分配策略对比 ==="
    echo
    
    # 分析heavy_computation函数的寄存器分配
    for file in "tests/RegisterTest_MiniC.s" "tests/RegisterTest_GCC_O0.s" "tests/RegisterTest_GCC_O1.s" "tests/RegisterTest_GCC_O2.s"; do
        if [ -f "$file" ]; then
            local name=$(basename "$file" .s)
            echo "     $name 的 heavy_computation 函数:"
            
            # 提取函数开头的寄存器分配模式
            awk '/heavy_computation:/{flag=1; count=0} flag && /^\s*[a-z]/ {count++; if(count<=8) print "       " $0} count>=8{flag=0}' "$file"
            echo
        fi
    done
}

detailed_comparison

echo
echo "=== 对比总结 ==="
echo "1. 代码质量对比："
echo "   - 指令数量：越少越好"
echo "   - 内存访问比例：越低表示寄存器利用越好"
echo "   - 寄存器使用数量：合理利用可用寄存器"
echo
echo "2. 性能对比："
echo "   - MiniC使用内置时间测量函数"
echo "   - GCC使用标准C库时间测量"
echo "   - 执行时间越短表示优化越好"
echo
echo "3. 寄存器分配策略："
echo "   - MiniC：图着色算法"
echo "   - GCC：成熟的寄存器分配优化"
echo
echo "生成的文件："
ls -la tests/RegisterTest_* 2>/dev/null | sed 's/^/  /'
