// 测试数组零初始化预处理功能

int main() {
    // 应该被处理：单个{0}初始化
    int a[3] = {0};
    
    // 应该被处理：多维数组单个{0}初始化
    int matrix[2][3][4] = {0};
    
    // 应该被处理：const数组
    const int const_arr[5] = {0};
    
    // 不应该被处理：复杂初始化
    int complex1[3] = {1, 2, 3};
    
    // 不应该被处理：嵌套{0}
    int complex2[2][2] = {{0}, {0}};
    
    // 不应该被处理：混合初始化
    int complex3[3] = {0, 1, 2};
    
    // 应该被处理：float数组
    float float_arr[4] = {0};
    
    return 0;
}
