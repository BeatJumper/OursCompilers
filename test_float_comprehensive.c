// 综合测试浮点数功能
float main() {
    // 1. 浮点数字面量
    float a = 3.14159;
    float b = 2.71828;
    float c = 0.0;
    float d = -1.5;
    
    // 2. 基本四则运算
    float sum = a + b;
    float diff = a - b;
    float product = a * b;
    float quotient = a / b;
    
    // 3. 混合类型运算
    int i = 10;
    float mixed1 = i + a;      // int + float
    float mixed2 = a - i;      // float - int
    float mixed3 = i * b;      // int * float
    float mixed4 = b / i;      // float / int
    
    // 4. 复杂表达式
    float complex1 = a + b * c - d;           // 运算优先级
    float complex2 = (a + b) / (c + 1.0);    // 括号和常量
    float complex3 = a * (b + c) - d / 2.0;  // 嵌套表达式
    
    // 5. 变量重新赋值
    a = sum + diff;
    b = product * quotient;
    c = mixed1 + mixed2;
    
    // 6. 连续运算
    float result = a + b + c + d + mixed3 + mixed4 + complex1 + complex2 + complex3;
    
    return result;
}
