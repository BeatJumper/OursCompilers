// 测试多维数组功能
int main() {
    // 二维数组声明
    int matrix[2][3];
    
    // 二维数组赋值
    matrix[0][0] = 1;
    matrix[0][1] = 2;
    matrix[0][2] = 3;
    matrix[1][0] = 4;
    matrix[1][1] = 5;
    matrix[1][2] = 6;
    
    // 读取二维数组元素
    int value = matrix[1][2];
    
    return value;  // 应该返回 6
}
