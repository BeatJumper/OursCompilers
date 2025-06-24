// 测试整数与浮点数的隐式类型转换
int main() {
    // 1. 整数字面量赋值给浮点数变量
    float a = 11;
    float b = 22;
    
    // 2. 混合类型运算
    int i = 5;
    float f = 2.5;
    
    float result1 = i + f;    // int + float -> float (i 转换为 float)
    float result2 = f - i;    // float - int -> float (i 转换为 float)
    float result3 = i * f;    // int * float -> float (i 转换为 float)
    float result4 = f / i;    // float / int -> float (i 转换为 float)
    
    // 3. 浮点数赋值给整数变量
    int j = f;                // float -> int (f 转换为 int)
    
    // 4. 函数返回值类型转换
    return result1;           // float -> int (result1 转换为 int)
}
