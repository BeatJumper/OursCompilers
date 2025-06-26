// 测试变量在函数中间声明的情况
int main() {
    // 第一组变量声明
    int a = 10;
    int b = 20;
    
    // 一些计算
    int sum1 = a + b;
    
    // 中间声明新变量
    int c = 30;
    int d = 40;
    
    // 更多计算
    int sum2 = c + d;
    
    // 在条件语句中声明变量
    if (sum1 > 25) {
        int temp = sum1 * 2;  // 这个变量应该只在if块中可见
        sum2 = sum2 + temp;
    }
    
    // 在循环中声明变量
    int i = 0;
    while (i < 3) {
        int loop_var = i * 10;  // 这个变量应该在每次循环中重新声明
        sum2 = sum2 + loop_var;
        i = i + 1;
    }
    
    // 最后的计算
    int final_result = sum1 + sum2;
    
    return final_result;
}
