// 测试浮点数加减乘除功能
float main() {
    float a = 3.14;
    float b = 2.5;
    float c = 1.0;

    // 基本运算
    float sum = a + b;        // 5.64
    float diff = a - b;       // 0.64
    float product = a * b;    // 7.85
    float quotient = a / b;   // 1.256

    // 复合运算
    float complex1 = a + b * c;     // 3.14 + 2.5 = 5.64
    float complex2 = (a - b) / c;   // (3.14 - 2.5) / 1.0 = 0.64
    float complex3 = a * b + c;     // 7.85 + 1.0 = 8.85

    // 变量重新赋值
    a = sum + diff;           // 5.64 + 0.64 = 6.28
    b = product - quotient;   // 7.85 - 1.256 = 6.594

    return a + b;             // 6.28 + 6.594 = 12.874
}
