// 测试多维数组定义时带初值
int main() {
    // 简单的二维数组初始化
    int matrix2x2[2][2] = {{1, 2}, {3, 4}};
    
    // 三维数组初始化
    int cube2x2x2[2][2][2] = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};
    
    // 浮点数二维数组初始化
    float fmatrix[2][3] = {{1.1, 2.2, 3.3}, {4.4, 5.5, 6.6}};
    
    // 混合类型的多维数组初始化
    int mixed[3][2] = {{10, 20}, {30, 40}, {50, 60}};
    
    // 访问测试
    int val1 = matrix2x2[0][0];  // 应该是 1
    int val2 = matrix2x2[1][1];  // 应该是 4
    int val3 = cube2x2x2[0][0][0];  // 应该是 1
    int val4 = cube2x2x2[1][1][1];  // 应该是 8
    float fval1 = fmatrix[0][1];  // 应该是 2.2
    float fval2 = fmatrix[1][2];  // 应该是 6.6
    int val5 = mixed[2][1];  // 应该是 60
    
    // 简单的运算测试
    int result = val1 + val2 + val3 + val4 + val5;
    
    return result;
}
