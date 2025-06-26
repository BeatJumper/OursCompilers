// 综合测试变量声明位置和作用域的正确性
int main() {
    // ===== 1. 函数开头的变量声明 =====
    int start_var1 = 100;
    float start_var2 = 3.14;
    
    // ===== 2. 中间穿插的变量声明 =====
    int calc1 = start_var1 * 2;  // 使用之前声明的变量
    
    // 在计算中间声明新变量
    int middle_var = 50;
    float middle_float = 2.5;
    
    int calc2 = calc1 + middle_var;  // 使用刚声明的变量
    
    // ===== 3. 条件语句中的变量声明 =====
    if (calc2 > 200) {
        // 这些变量只在if块中可见
        int if_local1 = 10;
        float if_local2 = 1.5;
        int if_result = if_local1 + calc2;
        
        // 嵌套的条件语句
        if (if_result > 250) {
            int nested_var = if_result / 2;
            calc2 = calc2 + nested_var;
        }
        
        calc2 = calc2 + if_local1;
    } else {
        // else块中的变量，与if块中的变量名可以相同
        int if_local1 = 20;  // 与if块中的同名变量不冲突
        calc2 = calc2 + if_local1;
    }
    
    // ===== 4. 循环中的变量声明 =====
    
    // for循环风格的while循环
    int loop_counter = 0;
    while (loop_counter < 3) {
        // 每次循环都会重新声明这些变量
        int loop_local = loop_counter * 10;
        float loop_float = loop_counter * 1.1;
        
        // 嵌套循环
        int inner_counter = 0;
        while (inner_counter < 2) {
            int inner_local = loop_local + inner_counter;
            calc2 = calc2 + inner_local;
            inner_counter = inner_counter + 1;
        }
        
        calc2 = calc2 + loop_local;
        loop_counter = loop_counter + 1;
    }
    
    // ===== 5. 复杂的嵌套作用域 =====
    int outer_scope = 1000;
    
    if (outer_scope > 500) {
        int level1_var = outer_scope / 2;
        
        if (level1_var > 400) {
            int level2_var = level1_var / 2;
            
            if (level2_var > 200) {
                int level3_var = level2_var / 2;
                outer_scope = outer_scope + level3_var;
            }
            
            outer_scope = outer_scope + level2_var;
        }
        
        outer_scope = outer_scope + level1_var;
    }
    
    // ===== 6. 变量声明与常量折叠结合 =====
    
    // 在函数末尾声明变量，测试常量折叠
    int const_test1 = 5 + 3;        // 应该折叠为 8
    int const_test2 = 10 * 4;       // 应该折叠为 40
    float const_test3 = 2.5 * 4.0;  // 应该折叠为 10.0
    
    // 使用之前计算的结果
    int final_calc = const_test1 + const_test2;
    
    // ===== 7. 混合类型的变量声明 =====
    
    // 整数和浮点数混合
    int mixed_int = 42;
    float mixed_float = 3.14;
    float mixed_result = mixed_int + mixed_float;  // 类型提升
    
    // ===== 8. 函数末尾的变量声明 =====
    
    // 在return之前声明变量
    int final_var1 = calc2 + final_calc;
    int final_var2 = outer_scope + mixed_int;
    int ultimate_result = final_var1 + final_var2;
    
    // 输出一些结果进行验证
    putint(start_var1);      // 100
    putint(middle_var);      // 50
    putint(const_test1);     // 8
    putint(const_test2);     // 40
    putfloat(const_test3);   // 10.0
    putfloat(mixed_result);  // 45.14
    
    return ultimate_result;
}
