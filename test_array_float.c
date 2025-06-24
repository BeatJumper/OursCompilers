// 测试浮点数数组功能
float main() {
    // 浮点数数组声明并初始化
    float arr[3] = {1.5, 2.5, 3.5};
    
    // 浮点数数组访问
    float value = arr[1];
    
    // 浮点数数组赋值
    arr[2] = 4.5;
    
    return value;  // 应该返回 2.5
}
