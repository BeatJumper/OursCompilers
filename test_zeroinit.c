// 测试zeroinitializer的情况
int arr2d[2][3] = {{1, 2}, {}}; // 第二行应该是zeroinitializer

int main() {
    return arr2d[0][0] + arr2d[1][0];
}
