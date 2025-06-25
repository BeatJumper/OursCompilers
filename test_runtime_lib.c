// SysY运行时库函数测试程序
int main() {
    // 测试输入函数
    int n = getint();
    int ch = getch();
    float f = getfloat();
    
    // 测试数组输入函数
    int arr[10];
    int count = getarray(arr);
    
    float farr[10];
    int fcount = getfarray(farr);
    
    // 测试输出函数
    putint(n);
    putch(ch);
    putfloat(f);
    
    // 测试数组输出函数
    putarray(count, arr);
    putfarray(fcount, farr);
    
    // 测试格式化输出函数
    putf("Hello %d %c %f\n", n, ch, f);
    
    // 测试计时函数（实际调用的是_sysy_starttime和_sysy_stoptime）
    _sysy_starttime(42);  // 模拟行号42
    // 一些计算
    int result = n + ch;
    _sysy_stoptime(45);   // 模拟行号45
    
    return result;
}
