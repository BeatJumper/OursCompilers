// 高级数学函数测试 - 模拟原始错误场景
const float eps = 1e-8;
const float pi = 3.14159;
const int max_iterations = 100;

float my_fabs(float x) {
    if (x > 0) return x;
    return -x;
}

float my_pow(float a, int n) {
    if (n < 0) return 1.0 / my_pow(a, -n);
    float res = 1.0;
    while (n) {
        if (n % 2) res = res * a;
        a = a * a;
        n = n / 2;
    }
    return res;
}

float my_sqrt(float x) {
    if (x > 100) return 10.0 * my_sqrt(x / 100);
    float t = (x / 8 + 0.5 + x * 2 + x / 4) / 4;
    int c = 10;
    while (c) {
        t = (t + x / t) / 2;
        c = c - 1;
    }
    return t;
}

float simpson(float a, float b, int flag) {
    float c = (b - a) / 2 + a;
    if (flag == 1) return (1.0/a + 4 * 1.0/c + 1.0/b) * (b - a) / 6;
    if (flag == 2) return (1.0/my_sqrt(1-a*a) + 4 * 1.0/my_sqrt(1-c*c) + 1.0/my_sqrt(1-b*b)) * (b - a) / 6;
    return 0;
}

float asr5(float a, float b, float eps, float A, int flag) {
    float c = (a + b) / 2;
    float L = simpson(a, c, flag), R = simpson(c, b, flag);
    if (my_fabs(L + R - A) <= 15 * eps) return L + R + (L + R - A) / 15.0;
    return asr5(a, c, eps / 2, L, flag) + asr5(c, b, eps / 2, R, flag);
}

float asr4(float a, float b, float eps, int flag) {
    return asr5(a, b, eps, simpson(a, b, flag), flag);
}

float my_exp(float x) {
    if (x > 1e-3) {
        float ee = my_exp(x / 2);
        return ee * ee;
    }
    return 1 + x + x * x / 2 + my_pow(x, 3) / 6 + my_pow(x, 4) / 24 + my_pow(x, 5) / 120;
}

int main() {
    float x = 2.5;
    float y = 1.5;
    
    // 测试各种数学函数
    float abs_result = my_fabs(x);
    float pow_result = my_pow(x, 2);
    float sqrt_result = my_sqrt(x);
    float exp_result = my_exp(x);
    
    // 测试积分函数
    float integral_result = asr4(1, x, eps, 1);
    
    // 使用常量
    float pi_val = pi;
    float eps_val = eps;
    int max_iter = max_iterations;
    
    return 0;
}
