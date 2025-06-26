// 常量折叠综合测试
int main() {
    // ===== 整数常量折叠测试 =====
    
    // 基本四则运算
    int add_result = 10 + 20;           // 应该折叠为 30
    int sub_result = 50 - 15;           // 应该折叠为 35
    int mul_result = 6 * 8;             // 应该折叠为 48
    int div_result = 100 / 4;           // 应该折叠为 25
    
    // 嵌套表达式
    int nested1 = (5 + 3) * (10 - 2);  // 应该折叠为 8 * 8 = 64
    int nested2 = (20 / 4) + (3 * 7);  // 应该折叠为 5 + 21 = 26
    int nested3 = ((2 + 3) * 4) - 1;   // 应该折叠为 (5 * 4) - 1 = 20 - 1 = 19
    
    // ===== 浮点数常量折叠测试 =====
    
    // 基本四则运算
    float fadd_result = 1.5 + 2.5;     // 应该折叠为 4.0
    float fsub_result = 10.0 - 3.5;    // 应该折叠为 6.5
    float fmul_result = 2.5 * 4.0;     // 应该折叠为 10.0
    float fdiv_result = 15.0 / 3.0;    // 应该折叠为 5.0
    
    // 嵌套浮点表达式
    float fnested1 = (1.5 + 2.5) * 2.0;        // 应该折叠为 4.0 * 2.0 = 8.0
    float fnested2 = (10.0 / 2.0) - 1.5;       // 应该折叠为 5.0 - 1.5 = 3.5
    float fnested3 = ((3.0 * 2.0) + 1.0) / 7.0; // 应该折叠为 (6.0 + 1.0) / 7.0 = 7.0 / 7.0 = 1.0
    
    // ===== 混合类型常量折叠测试 =====
    
    // 整数与浮点数混合
    float mixed1 = 5 + 2.5;            // 应该折叠为 7.5
    float mixed2 = 10 - 1.5;           // 应该折叠为 8.5
    float mixed3 = 3 * 2.5;            // 应该折叠为 7.5
    float mixed4 = 20 / 4.0;           // 应该折叠为 5.0
    
    // 复杂混合表达式
    float complex1 = (2 + 3.0) * (4 - 1.5);    // 应该折叠为 5.0 * 2.5 = 12.5
    float complex2 = (10 / 2) + (3.5 * 2);     // 应该折叠为 5.0 + 7.0 = 12.0
    
    // ===== 边界情况测试 =====
    
    // 零值运算
    int zero_add = 0 + 42;             // 应该折叠为 42
    int zero_mul = 0 * 999;            // 应该折叠为 0
    float zero_fadd = 0.0 + 3.14;      // 应该折叠为 3.14
    float zero_fmul = 0.0 * 100.0;     // 应该折叠为 0.0
    
    // 一值运算
    int one_mul = 1 * 42;              // 应该折叠为 42
    int one_div = 42 / 1;              // 应该折叠为 42
    float one_fmul = 1.0 * 3.14;       // 应该折叠为 3.14
    float one_fdiv = 3.14 / 1.0;       // 应该折叠为 3.14
    
    // 负数运算
    int neg_add = (-5) + 10;           // 应该折叠为 5
    int neg_mul = (-3) * 4;            // 应该折叠为 -12
    float neg_fadd = (-2.5) + 7.5;     // 应该折叠为 5.0
    float neg_fmul = (-1.5) * 2.0;     // 应该折叠为 -3.0
    
    // 输出一些结果进行验证
    putint(add_result);     // 应该输出 30
    putint(nested1);        // 应该输出 64
    putfloat(fadd_result);  // 应该输出 4.0
    putfloat(mixed1);       // 应该输出 7.5
    
    return add_result + nested1;  // 应该返回 30 + 64 = 94
}
