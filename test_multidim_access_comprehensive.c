// 全面测试多维数组访问
int main() {
    // 声明各种维度的数组
    int matrix2d[3][4];
    int cube3d[2][3][4];
    int hyper4d[2][2][3][4];
    float fmatrix[2][3];
    
    // 二维数组访问和赋值
    matrix2d[0][0] = 10;
    matrix2d[1][2] = 20;
    matrix2d[2][3] = 30;
    
    // 三维数组访问和赋值
    cube3d[0][1][2] = 100;
    cube3d[1][0][3] = 200;
    cube3d[1][2][1] = 300;
    
    // 四维数组访问和赋值
    hyper4d[0][1][2][3] = 1000;
    hyper4d[1][0][1][2] = 2000;
    
    // 浮点数二维数组访问
    fmatrix[0][1] = 3.14;
    fmatrix[1][2] = 2.71;
    
    // 读取测试
    int val1 = matrix2d[1][2];
    int val2 = cube3d[1][2][1];
    int val3 = hyper4d[1][0][1][2];
    float fval = fmatrix[1][2];
    
    // 复杂表达式中的数组访问
    int result = matrix2d[0][0] + cube3d[0][1][2] + hyper4d[0][1][2][3];
    
    return result;
}
