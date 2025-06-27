// 复杂数学函数测试 - 包含浮点数常量
const float PI = 3.14159;
const float E = 2.71828;
const float EPS = 1e-6;
const int MAX_ITER = 100;

// 绝对值函数
float my_fabs(float x) {
    if (x > 0.0) return x;
    return -x;
}

// 幂函数
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

// 平方根函数
float my_sqrt(float x) {
    if (x > 100.0) return 10.0 * my_sqrt(x / 100.0);
    float t = (x / 8.0 + 0.5 + x * 2.0 + x / 4.0) / 4.0;
    int c = 10;
    while (c) {
        t = (t + x / t) / 2.0;
        c = c - 1;
    }
    return t;
}

// 简单的数学计算
float calc_circle_area(float radius) {
    return PI * radius * radius;
}

float calc_exp_approx(float x) {
    // e^x 的泰勒级数近似
    return 1.0 + x + x*x/2.0 + my_pow(x, 3)/6.0;
}

int main() {
    float radius = 5.0;
    float area = calc_circle_area(radius);
    
    float x = 2.0;
    float exp_val = calc_exp_approx(x);
    
    float sqrt_val = my_sqrt(25.0);
    float abs_val = my_fabs(-3.14);
    
    // 使用常量
    float pi_val = PI;
    float e_val = E;
    float eps_val = EPS;
    int max_iter = MAX_ITER;
    
    return 0;
}
