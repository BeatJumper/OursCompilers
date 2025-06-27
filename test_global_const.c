// 测试全局常量声明，包括整数和浮点数
const int global_int = 42;
const float global_float = 3.14;
const int zero_int = 0;
const float zero_float = 0.0;

int main() {
    // 使用全局常量
    int a = global_int;
    float b = global_float;
    int c = zero_int;
    float d = zero_float;
    
    // 简单计算
    int result = a + c;
    float fresult = b + d;
    
    return result;
}
