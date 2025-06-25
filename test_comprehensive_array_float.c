// 全面测试数组与浮点数功能
int main() {
    // ========== 一维数组测试 ==========
    
    // 1. 整数一维数组
    int arr1[5] = {1, 2, 3, 4, 5};
    
    // 2. 浮点数一维数组
    float farr1[4] = {1.1, 2.2, 3.3, 4.4};
    
    // 3. 混合访问和运算
    int val1 = arr1[0] + arr1[4];  // 1 + 5 = 6
    float fval1 = farr1[1] + farr1[3];  // 2.2 + 4.4 = 6.6
    
    // ========== 二维数组测试 ==========
    
    // 4. 整数二维数组
    int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    // 5. 浮点数二维数组
    float fmatrix[2][4] = {{1.0, 2.0, 3.0, 4.0}, {5.0, 6.0, 7.0, 8.0}};
    
    // 6. 二维数组访问和运算
    int val2 = matrix[0][0] + matrix[2][2];  // 1 + 9 = 10
    float fval2 = fmatrix[0][1] + fmatrix[1][3];  // 2.0 + 8.0 = 10.0
    
    // ========== 三维数组测试 ==========
    
    // 7. 整数三维数组
    int cube[2][2][3] = {{{1, 2, 3}, {4, 5, 6}}, {{7, 8, 9}, {10, 11, 12}}};
    
    // 8. 浮点数三维数组
    float fcube[2][2][2] = {{{1.1, 2.2}, {3.3, 4.4}}, {{5.5, 6.6}, {7.7, 8.8}}};
    
    // 9. 三维数组访问和运算
    int val3 = cube[0][0][0] + cube[1][1][2];  // 1 + 12 = 13
    float fval3 = fcube[0][1][0] + fcube[1][0][1];  // 3.3 + 6.6 = 9.9
    
    // ========== 类型转换测试 ==========
    
    // 10. 整数数组元素转浮点数
    float converted1 = arr1[2];  // 3 -> 3.0
    
    // 11. 浮点数数组元素转整数
    int converted2 = farr1[0];  // 1.1 -> 1
    
    // 12. 混合类型运算
    float mixed1 = arr1[1] + farr1[2];  // 2 + 3.3 = 5.3
    int mixed2 = farr1[1] + arr1[3];  // 2.2 + 4 = 6
    
    // ========== 复杂嵌套访问测试 ==========
    
    // 13. 使用数组元素作为索引
    int indices[3] = {0, 1, 2};
    int nested_val1 = matrix[indices[0]][indices[2]];  // matrix[0][2] = 3
    float nested_val2 = fmatrix[indices[1]][indices[0]];  // fmatrix[1][0] = 5.0
    
    // 14. 多层嵌套访问
    int coord[2] = {1, 0};
    int deep_val = cube[coord[0]][coord[1]][indices[coord[0]]];  // cube[1][0][1] = 8
    
    // ========== 表达式索引测试 ==========
    
    // 15. 算术表达式作为索引
    int expr_val1 = matrix[indices[0] + 1][indices[1] - 1];  // matrix[1][0] = 4
    float expr_val2 = fcube[coord[1]][coord[0]][indices[0] + coord[1]];  // fcube[0][1][1] = 4.4
    
    // ========== 综合运算测试 ==========
    
    // 16. 多维数组元素的复杂运算
    float complex_calc = (matrix[0][0] + fmatrix[0][0]) * (cube[0][0][1] + fcube[0][0][1]);
    // (1 + 1.0) * (2 + 2.2) = 2.0 * 4.2 = 8.4
    
    // 17. 数组元素的连续运算
    int sum_int = arr1[0] + arr1[1] + arr1[2] + arr1[3] + arr1[4];  // 1+2+3+4+5 = 15
    float sum_float = farr1[0] + farr1[1] + farr1[2] + farr1[3];  // 1.1+2.2+3.3+4.4 = 11.0
    
    // ========== 返回值测试 ==========
    
    // 18. 综合结果计算
    int final_int = val1 + val2 + val3 + converted2 + mixed2 + nested_val1 + deep_val + expr_val1 + sum_int;
    float final_float = fval1 + fval2 + fval3 + converted1 + mixed1 + nested_val2 + expr_val2 + complex_calc + sum_float;
    
    // 19. 最终类型转换和返回
    int result = final_int + final_float;  // 整数 + 浮点数 -> 整数
    
    return result;
}
