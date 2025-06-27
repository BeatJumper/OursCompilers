int main() {
    // 创建大量局部变量来产生大偏移量
    int arr1[1000];
    int arr2[1000]; 
    int arr3[1000];
    
    // 使用这些数组
    arr1[0] = 1;
    arr2[0] = 2;
    arr3[0] = 3;
    
    return arr1[0] + arr2[0] + arr3[0];
}
