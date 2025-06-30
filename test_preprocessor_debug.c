// 简单测试预处理器功能

int main() {
    // 应该被处理：去掉 = {0}
    int simple[3] = {0};
    
    // 应该被处理：多维数组
    int array2d[2][3] = {0};
    
    // 不应该被处理：复杂初始化
    int complex[2] = {1, 2};
    
    // 测试简单赋值和访问
    simple[0] = 100;
    array2d[0][0] = 200;
    
    putint(simple[0]);
    putch(10);
    putint(array2d[0][0]);
    putch(10);
    
    return 0;
}
