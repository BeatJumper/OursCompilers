// 综合功能测试：运行时库、浮点数、数组、常量折叠
int main() {
    // ===== 1. 常量折叠测试 =====
    
    // 整数常量折叠
    int const_add = 3 + 5;        // 应该折叠为 8
    int const_mul = 6 * 7;        // 应该折叠为 42
    
    // 浮点数常量折叠
    float const_fadd = 1.5 + 2.5;  // 应该折叠为 4.0
    float const_fmul = 2.0 * 3.0;  // 应该折叠为 6.0
    
    // 混合类型常量折叠
    float const_mixed = 3 + 2.5;    // 应该折叠为 5.5
    
    // ===== 2. 多维数组测试 =====
    
    // 二维整数数组（应该生成完全展开的全局常量）
    int matrix_int[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    // 二维浮点数组（应该生成完全展开的全局常量）
    float matrix_float[2][3] = {{1.1, 2.2, 3.3}, {4.4, 5.5, 6.6}};
    
    // ===== 3. SysY运行时库函数测试 =====
    
    // 输入函数测试
    int input_int = getint();
    float input_float = getfloat();
    
    // 输出函数测试
    putint(const_add);
    putfloat(const_fadd);
    
    // 计时函数测试
    _sysy_starttime(100);  // 开始计时
    
    // 一些计算操作
    int result = const_add + const_mul;
    float fresult = const_fadd + const_fmul + const_mixed;
    
    // 数组访问测试
    int access_test = matrix_int[1][2];
    float faccess_test = matrix_float[0][1];
    
    _sysy_stoptime(110);   // 结束计时
    
    return result;
}
