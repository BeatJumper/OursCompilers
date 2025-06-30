// 简单的多维数组传参测试程序

// 测试二维数组传参
int sum_2d(int arr[][3], int rows) {
    int total = 0;
    int i = 0;

    while (i < rows) {
        int j = 0;
        while (j < 3) {
            total = total + arr[i][j];
            j = j + 1;
        }
        i = i + 1;
    }

    return total;
}

// 测试三维数组传参
int sum_3d(int arr[][2][3], int layers) {
    int total = 0;
    int i = 0;

    while (i < layers) {
        int j = 0;
        while (j < 2) {
            int k = 0;
            while (k < 3) {
                total = total + arr[i][j][k];
                k = k + 1;
            }
            j = j + 1;
        }
        i = i + 1;
    }

    return total;
}

int main() {
    // 二维数组初始化
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    // 三维数组初始化
    int cube[2][2][3] = {
        {{1, 2, 3}, {4, 5, 6}},
        {{7, 8, 9}, {10, 11, 12}}
    };
    
    // 测试二维数组传参
    int result2d = sum_2d(matrix, 2);
    putint(result2d);  // 期望输出: 21 (1+2+3+4+5+6)
    putch(10);         // 换行
    
    // 测试三维数组传参
    int result3d = sum_3d(cube, 2);
    putint(result3d);  // 期望输出: 78 (1+2+...+12)
    putch(10);         // 换行
    
    return 0;
}
