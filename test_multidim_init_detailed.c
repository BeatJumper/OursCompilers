// 详细测试多维数组初始化
int main() {
    // 简单的二维数组初始化
    int matrix2x2[2][2] = {{1, 2}, {3, 4}};
    
    // 三维数组初始化
    int cube2x2x2[2][2][2] = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};
    
    // 浮点数二维数组初始化
    float fmatrix[2][3] = {{1.1, 2.2, 3.3}, {4.4, 5.5, 6.6}};
    
    // 访问测试
    int val1 = matrix2x2[1][1];
    int val2 = cube2x2x2[1][1][1];
    float fval = fmatrix[1][2];
    
    return val1 + val2;
}
