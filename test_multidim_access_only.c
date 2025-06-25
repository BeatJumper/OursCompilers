// 专门测试多维数组访问（不涉及初始化）
int main() {
    // 声明多维数组（不初始化）
    int matrix2d[3][4];
    int cube3d[2][3][4];
    float fmatrix[2][3];
    
    // 二维数组访问和赋值
    matrix2d[0][0] = 1;
    matrix2d[1][2] = 5;
    matrix2d[2][3] = 9;
    
    // 三维数组访问和赋值
    cube3d[0][1][2] = 10;
    cube3d[1][2][3] = 20;
    
    // 浮点数二维数组访问
    fmatrix[0][1] = 3.14;
    fmatrix[1][2] = 2.71;
    
    // 多维数组读取
    int val1 = matrix2d[1][2];
    int val2 = cube3d[1][2][3];

    return val1 + val2;
}
