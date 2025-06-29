// 测试多维数组参数的处理
void test1(int arr[]) {
    // 一维数组参数：退化为 i32*
    arr[0] = 1;
}

void test2(int matrix[][4]) {
    // 二维数组参数：退化为 [4 x i32]*
    matrix[0][0] = 2;
}

void test3(int cube[][3][4]) {
    // 三维数组参数：退化为 [3 x [4 x i32]]*
    cube[0][1][2] = 3;
}

int main() {
    int arr[5];
    int matrix[2][4];
    int cube[2][3][4];
    
    test1(arr);
    test2(matrix);
    test3(cube);
    
    return 0;
}
