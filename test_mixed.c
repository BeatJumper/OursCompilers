// 测试整数和浮点数混合运算
float main() {
    int i = 5;
    float f = 2.5;
    
    // 混合运算 - 应该转换为浮点运算
    float result1 = i + f;    // int + float -> float
    float result2 = f - i;    // float - int -> float  
    float result3 = i * f;    // int * float -> float
    float result4 = f / i;    // float / int -> float
    
    return result1 + result2 + result3 + result4;
}
