// 简单的浮点数测试
const float PI = 3.14159;
const int MAX_VAL = 100;

float simple_add(float a, float b) {
    return a + b;
}

float use_constants() {
    float result = PI * 2.0;
    return result;
}

int main() {
    float x = 1.5;
    float y = 2.5;
    float sum = simple_add(x, y);
    float const_result = use_constants();
    int max_val = MAX_VAL;
    
    return 0;
}
