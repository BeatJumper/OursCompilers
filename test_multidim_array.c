// 测试多维数组函数传参和预处理器零初始化功能

// 处理二维数组的函数
int process2D(int arr[][3], int rows) {
    int sum = 0;
    int i = 0;
    while (i < rows) {
        int j = 0;
        while (j < 3) {
            sum = sum + arr[i][j];
            j = j + 1;
        }
        i = i + 1;
    }
    return sum;
}

// 处理三维数组的函数
int process3D(int arr[][2][2], int depth) {
    int sum = 0;
    int i = 0;
    while (i < depth) {
        int j = 0;
        while (j < 2) {
            int k = 0;
            while (k < 2) {
                sum = sum + arr[i][j][k];
                k = k + 1;
            }
            j = j + 1;
        }
        i = i + 1;
    }
    return sum;
}

int main() {
    // 测试预处理器功能：这些应该被处理，去掉 = {0}
    int array2d[2][3] = {0};
    int array3d[3][2][2] = {0};
    
    // 这些不应该被处理（复杂初始化）
    int complex2d[2][2] = {{1, 2}, {3, 4}};
    
    // 手动初始化一些值来测试
    array2d[0][0] = 1;
    array2d[0][1] = 2;
    array2d[0][2] = 3;
    array2d[1][0] = 4;
    array2d[1][1] = 5;
    array2d[1][2] = 6;
    
    array3d[0][0][0] = 10;
    array3d[0][0][1] = 20;
    array3d[0][1][0] = 30;
    array3d[0][1][1] = 40;
    array3d[1][0][0] = 50;
    array3d[1][0][1] = 60;
    array3d[1][1][0] = 70;
    array3d[1][1][1] = 80;
    array3d[2][0][0] = 90;
    array3d[2][0][1] = 100;
    array3d[2][1][0] = 110;
    array3d[2][1][1] = 120;
    
    // 调用函数处理数组并打印结果
    int sum2d = process2D(array2d, 2);
    putint(sum2d);  // 应该输出 21 (1+2+3+4+5+6)
    putch(10);      // 换行符
    
    int sum3d = process3D(array3d, 3);
    putint(sum3d);  // 应该输出 780 (10+20+...+120)
    putch(10);      // 换行符
    
    // 测试复杂初始化的数组
    int complex_sum = complex2d[0][0] + complex2d[0][1] + complex2d[1][0] + complex2d[1][1];
    putint(complex_sum);  // 应该输出 10 (1+2+3+4)
    putch(10);      // 换行符
    
    return 0;
}
