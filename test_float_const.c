// 测试浮点数常量处理
const float PI = 3.14159;
const float E = 2.71828;
const int MAX_ITER = 100;

float simple_calc(float x) {
    float result = PI * x + E;
    return result;
}

int main() {
    float a = 1.0;
    float b = 2.5;
    float c = simple_calc(a);
    
    // 使用常量
    float pi_val = PI;
    float e_val = E;
    int max_val = MAX_ITER;
    
    return 0;
}
