// 测试常数的内部数据结构
int main() {
    int a = 10;
    int b = a % 3;  // 这里的常数3应该是ConstInt对象
    float c = 2.5;  // 这里的常数2.5应该是ConstFloat对象
    int d = a + 7;  // 这里的常数7应该是ConstInt对象
    return b + d;
}
