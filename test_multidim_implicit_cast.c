// 测试多维数组的隐式类型转换
int main() {
    // 声明多维数组
    int imatrix[2][3];
    float fmatrix[2][3];
    
    // 1. 整数字面量赋值给多维数组元素
    imatrix[0][0] = 10;
    fmatrix[0][0] = 20;  // int -> float 隐式转换
    
    // 2. 浮点数字面量赋值给多维数组元素
    imatrix[0][1] = 3.14;  // float -> int 隐式转换
    fmatrix[0][1] = 2.71;
    
    // 3. 多维数组元素之间的赋值
    imatrix[1][0] = imatrix[0][0];  // int -> int
    fmatrix[1][0] = fmatrix[0][0];  // float -> float
    
    // 4. 混合类型赋值
    imatrix[1][1] = fmatrix[0][1];  // float -> int 隐式转换
    fmatrix[1][1] = imatrix[0][1];  // int -> float 隐式转换
    
    // 5. 多维数组元素参与运算
    int result1 = imatrix[0][0] + imatrix[1][0];  // int + int
    float result2 = fmatrix[0][0] + fmatrix[1][0];  // float + float
    
    // 6. 混合类型运算
    int result3 = imatrix[0][0] + fmatrix[0][0];  // int + float -> int (需要转换)
    float result4 = imatrix[0][0] + fmatrix[0][0];  // int + float -> float
    
    return result1;
}
