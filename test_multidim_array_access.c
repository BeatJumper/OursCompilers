// 测试多维数组访问方式
int main() {
    // 二维数组声明
    int matrix[3][4];
    
    // 多维数组访问和赋值
    matrix[0][0] = 1;
    matrix[1][2] = 5;
    matrix[2][3] = 9;
    
    // 多维数组读取
    int value1 = matrix[0][0];
    int value2 = matrix[1][2];
    int value3 = matrix[2][3];
    
    // 三维数组测试
    int cube[2][3][4];
    cube[1][2][3] = 42;
    int deep_value = cube[1][2][3];
    
    return value1 + value2 + value3 + deep_value;
}
