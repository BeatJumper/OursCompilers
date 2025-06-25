// 综合功能测试程序
int main() {
    // ===== 1. 常量折叠测试 =====
    
    // 整数常量折叠
    int const_add = 3 + 5;        // 应该折叠为 8
    int const_sub = 10 - 4;       // 应该折叠为 6
    int const_mul = 6 * 7;        // 应该折叠为 42
    int const_div = 20 / 4;       // 应该折叠为 5
    
    // 浮点数常量折叠
    float const_fadd = 1.5 + 2.5;  // 应该折叠为 4.0
    float const_fsub = 5.0 - 1.5;  // 应该折叠为 3.5
    float const_fmul = 2.0 * 3.0;  // 应该折叠为 6.0
    float const_fdiv = 8.0 / 2.0;  // 应该折叠为 4.0
    
    // 混合类型常量折叠
    float const_mixed1 = 3 + 2.5;    // 应该折叠为 5.5
    float const_mixed2 = 10 - 1.5;   // 应该折叠为 8.5
    float const_mixed3 = 4 * 2.5;    // 应该折叠为 10.0
    float const_mixed4 = 15 / 3.0;   // 应该折叠为 5.0
    
    // 嵌套表达式常量折叠
    int nested_const = (2 + 3) * (4 - 1);  // 应该折叠为 5 * 3 = 15
    
    // ===== 2. 多维数组初始化测试 =====
    
    // 二维整数数组（应该生成完全展开的全局常量）
    int matrix_int[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    // 二维浮点数组（应该生成完全展开的全局常量）
    float matrix_float[2][3] = {{1.1, 2.2, 3.3}, {4.4, 5.5, 6.6}};
    
    // 三维数组测试
    int cube[2][2][2] = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};
    
    // ===== 3. SysY运行时库函数测试 =====
    
    // 输入函数测试
    int input_int = getint();
    int input_char = getch();
    float input_float = getfloat();
    
    // 数组输入函数测试
    int input_array[5];
    int array_count = getarray(input_array);
    
    float input_farray[5];
    int farray_count = getfarray(input_farray);
    
    // 输出函数测试
    putint(const_add);
    putch(65);  // 输出字符 'A'
    putfloat(const_fadd);
    
    // 数组输出函数测试
    putarray(3, matrix_int[0]);  // 输出第一行
    putfarray(3, matrix_float[1]); // 输出第二行
    
    // 计时函数测试
    _sysy_starttime(100);  // 开始计时
    
    // 一些计算操作
    int result = const_add + const_mul + nested_const;
    float fresult = const_fadd + const_fmul + const_mixed1;
    
    // 数组访问测试
    int access_test = matrix_int[1][2] + cube[0][1][1];
    float faccess_test = matrix_float[0][1] + matrix_float[1][0];
    
    _sysy_stoptime(110);   // 结束计时
    
    // ===== 4. 综合计算测试 =====
    
    // 结合常量折叠和数组访问
    int final_result = (5 + 3) * matrix_int[0][0] + access_test;
    
    return final_result;
}
