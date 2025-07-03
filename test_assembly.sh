#!/bin/bash

# ARM64汇编测试脚本
# 功能：编译C源码到ARM64汇编，链接运行时库，生成可执行文件并测试

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查必要的工具
check_tools() {
    print_info "检查必要的工具..."
    
    if ! command -v aarch64-linux-gnu-gcc &> /dev/null; then
        print_error "aarch64-linux-gnu-gcc 未找到，请安装 ARM64 交叉编译工具链"
        exit 1
    fi
    
    if ! command -v qemu-aarch64-static &> /dev/null; then
        print_error "qemu-aarch64-static 未找到，请安装 QEMU 用户模式模拟器"
        exit 1
    fi
    
    if [ ! -f "./build/minic" ]; then
        print_error "minic 编译器未找到，请先构建项目"
        exit 1
    fi
    
    print_success "所有必要工具检查完成"
}

# 编译运行时库
compile_runtime() {
    print_info "编译运行时库 std.c..."
    
    if [ ! -f "ir/std.c" ]; then
        print_error "运行时库 ir/std.c 未找到"
        exit 1
    fi
    
    aarch64-linux-gnu-gcc -c -o ir/std.o ir/std.c
    
    if [ $? -eq 0 ]; then
        print_success "运行时库编译完成: ir/std.o"
    else
        print_error "运行时库编译失败"
        exit 1
    fi
}

# 使用minic编译器生成汇编代码
compile_to_assembly() {
    local source_file="$1"
    local output_file="$2"
    
    print_info "使用 minic 编译器生成 ARM64 汇编代码..."
    print_info "源文件: $source_file"
    print_info "输出文件: $output_file"
    
    if [ ! -f "$source_file" ]; then
        print_error "源文件 $source_file 未找到"
        exit 1
    fi
    
    ./build/minic -S -o "$output_file" "$source_file"
    
    if [ $? -eq 0 ]; then
        print_success "汇编代码生成完成: $output_file"
    else
        print_error "汇编代码生成失败"
        exit 1
    fi
}

# 链接生成可执行文件
link_executable() {
    local assembly_file="$1"
    local executable_file="$2"
    
    print_info "链接生成可执行文件..."
    print_info "汇编文件: $assembly_file"
    print_info "可执行文件: $executable_file"
    
    if [ ! -f "$assembly_file" ]; then
        print_error "汇编文件 $assembly_file 未找到"
        exit 1
    fi
    
    if [ ! -f "ir/std.o" ]; then
        print_error "运行时库 ir/std.o 未找到，请先编译运行时库"
        exit 1
    fi
    
    aarch64-linux-gnu-gcc -static -g -o "$executable_file" "$assembly_file" ir/std.o
    
    if [ $? -eq 0 ]; then
        print_success "可执行文件生成完成: $executable_file"
    else
        print_error "链接失败"
        exit 1
    fi
}

# 运行可执行文件并对比结果
run_and_compare() {
    local executable_file="$1"
    local input_file="$2"
    local expected_output_file="$3"
    
    print_info "运行可执行文件并对比结果..."
    
    if [ ! -f "$executable_file" ]; then
        print_error "可执行文件 $executable_file 未找到"
        exit 1
    fi
    
    if [ ! -f "$input_file" ]; then
        print_error "输入文件 $input_file 未找到"
        exit 1
    fi
    
    if [ ! -f "$expected_output_file" ]; then
        print_error "期望输出文件 $expected_output_file 未找到"
        exit 1
    fi
    
    # 创建临时输出文件
    local actual_output_file="$(dirname "$executable_file")/actual_output.tmp"
    
    print_info "运行程序: qemu-aarch64-static $executable_file < $input_file"
    
    # 运行程序并捕获输出
    if qemu-aarch64-static "$executable_file" < "$input_file" > "$actual_output_file" 2>/dev/null; then
        local exit_code=$?
        print_success "程序运行完成，退出码: $exit_code"
        
        # 显示实际输出
        print_info "实际输出:"
        cat "$actual_output_file"
        
        # 显示期望输出
        print_info "期望输出:"
        cat "$expected_output_file"
        
        # 对比输出
        if diff -q "$actual_output_file" "$expected_output_file" > /dev/null; then
            print_success "✅ 输出对比成功！程序运行正确"
        else
            print_error "❌ 输出对比失败！"
            print_info "详细差异:"
            diff "$actual_output_file" "$expected_output_file" || true
        fi
        
        # 清理临时文件
        rm -f "$actual_output_file"
        
    else
        print_error "程序运行失败"
        rm -f "$actual_output_file"
        exit 1
    fi
}

# 主函数
main() {
    print_info "开始 ARM64 汇编测试流程..."
    
    # 默认参数
    local source_file="./tests/AddFunction.c"
    local assembly_file="./tests/AddFunction.s"
    local executable_file="./tests/AddFunction_test"
    local input_file="./tests/add.in"
    local expected_output_file="./tests/add.out"
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -s|--source)
                source_file="$2"
                shift 2
                ;;
            -a|--assembly)
                assembly_file="$2"
                shift 2
                ;;
            -e|--executable)
                executable_file="$2"
                shift 2
                ;;
            -i|--input)
                input_file="$2"
                shift 2
                ;;
            -o|--output)
                expected_output_file="$2"
                shift 2
                ;;
            -h|--help)
                echo "用法: $0 [选项]"
                echo "选项:"
                echo "  -s, --source FILE        源文件路径 (默认: ./tests/AddFunction.c)"
                echo "  -a, --assembly FILE      汇编文件路径 (默认: ./tests/AddFunction.s)"
                echo "  -e, --executable FILE    可执行文件路径 (默认: ./tests/AddFunction_test)"
                echo "  -i, --input FILE         输入文件路径 (默认: ./tests/add.in)"
                echo "  -o, --output FILE        期望输出文件路径 (默认: ./tests/add.out)"
                echo "  -h, --help               显示此帮助信息"
                exit 0
                ;;
            *)
                print_error "未知选项: $1"
                echo "使用 -h 或 --help 查看帮助信息"
                exit 1
                ;;
        esac
    done
    
    print_info "使用参数:"
    print_info "  源文件: $source_file"
    print_info "  汇编文件: $assembly_file"
    print_info "  可执行文件: $executable_file"
    print_info "  输入文件: $input_file"
    print_info "  期望输出文件: $expected_output_file"
    
    # 执行测试流程
    check_tools
    compile_runtime
    compile_to_assembly "$source_file" "$assembly_file"
    link_executable "$assembly_file" "$executable_file"
    run_and_compare "$executable_file" "$input_file" "$expected_output_file"
    
    print_success "🎉 ARM64 汇编测试流程完成！"
}

# 运行主函数
main "$@"
