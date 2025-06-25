// 多维数组赋初值的测试程序
int main() {
    // 一维整数数组（未初始化）
    int int_arr[5];
    
    // 二维整数数组（带初值）
    int int_arr2[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    // 一维浮点数组（未初始化）
    float float_arr[5];
    
    // 二维浮点数组（带初值）
    float float_arr2[3][3] = {{1.1, 2.2, 3.3}, {4.4, 5.5, 6.6}, {7.7, 8.8, 9.9}};
    
    // 访问和使用数组元素
    int sum_int = int_arr2[0][0] + int_arr2[1][1] + int_arr2[2][2];  // 对角线和: 1+5+9=15
    float sum_float = float_arr2[0][0] + float_arr2[1][1] + float_arr2[2][2];  // 对角线和: 1.1+5.5+9.9=16.5
    
    // 给未初始化的数组赋值
    int_arr[0] = 10;
    int_arr[1] = 20;
    int_arr[2] = 30;
    int_arr[3] = 40;
    int_arr[4] = 50;
    
    float_arr[0] = 1.5;
    float_arr[1] = 2.5;
    float_arr[2] = 3.5;
    float_arr[3] = 4.5;
    float_arr[4] = 5.5;
    
    // 计算一维数组的和
    int sum1d_int = int_arr[0] + int_arr[1] + int_arr[2] + int_arr[3] + int_arr[4];  // 10+20+30+40+50=150
    float sum1d_float = float_arr[0] + float_arr[1] + float_arr[2] + float_arr[3] + float_arr[4];  // 1.5+2.5+3.5+4.5+5.5=17.5
    
    // 修改二维数组的值
    int_arr2[1][1] = 100;  // 将中心元素从5改为100
    float_arr2[1][1] = 100.5;  // 将中心元素从5.5改为100.5
    
    // 重新计算对角线和
    int new_sum_int = int_arr2[0][0] + int_arr2[1][1] + int_arr2[2][2];  // 1+100+9=110
    float new_sum_float = float_arr2[0][0] + float_arr2[1][1] + float_arr2[2][2];  // 1.1+100.5+9.9=111.5
    
    // 复杂的数组访问
    int complex_access = int_arr2[0][2] + int_arr2[2][0];  // 3+7=10
    float complex_access_f = float_arr2[0][2] + float_arr2[2][0];  // 3.3+7.7=11.0
    
    // 使用数组元素作为索引
    int idx1 = 1;
    int idx2 = 2;
    int indexed_value = int_arr2[idx1][idx2];  // int_arr2[1][2] = 6
    float indexed_value_f = float_arr2[idx1][idx2];  // float_arr2[1][2] = 6.6
    
    // 最终结果计算
    int final_result = sum_int + sum1d_int + new_sum_int + complex_access + indexed_value;
    // 15 + 150 + 110 + 10 + 6 = 291
    
    return final_result;
}
