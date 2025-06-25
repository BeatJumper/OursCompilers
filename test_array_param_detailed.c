// 详细测试数组传参
void print1D(int arr[]) {
    return;
}

void print2D(int matrix[][4]) {
    return;
}

void print3D(int cube[][3][4]) {
    return;
}

void printFloat2D(float fmatrix[][3]) {
    return;
}

int main() {
    // 声明各种数组
    int arr1d[5];
    int matrix2d[2][4];
    int cube3d[2][3][4];
    float fmatrix[2][3];
    
    // 测试数组传参
    print1D(arr1d);
    print2D(matrix2d);
    print3D(cube3d);
    printFloat2D(fmatrix);
    
    return 0;
}
