int helper(int x) {
    return x + 1;
}

int main() {
    // 创建大量局部数组来增加栈帧大小
    int arr1[50];
    int arr2[50]; 
    int arr3[50];
    int arr4[50];
    int arr5[50];
    
    // 初始化一些数组元素
    arr1[0] = 1;
    arr2[0] = 2;
    arr3[0] = 3;
    arr4[0] = 4;
    arr5[0] = 5;
    
    // 调用函数以确保需要保存FP和LR
    int result = helper(arr1[0] + arr2[0] + arr3[0] + arr4[0] + arr5[0]);
    
    return result;
}
