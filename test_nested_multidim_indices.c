// 测试多维数组下标表达式本身也是多维数组访问
int main() {
    // 声明各种多维数组
    int matrix[3][4];           // 主要的二维数组
    int cube[2][3][4];          // 主要的三维数组
    int indices2d[2][3];        // 二维索引数组
    int indices3d[2][2][2];     // 三维索引数组
    
    // 初始化索引数组
    indices2d[0][0] = 1;
    indices2d[0][1] = 2;
    indices2d[1][0] = 0;
    indices2d[1][1] = 3;
    
    indices3d[0][0][0] = 1;
    indices3d[0][0][1] = 2;
    indices3d[0][1][0] = 0;
    indices3d[0][1][1] = 1;
    indices3d[1][0][0] = 1;
    indices3d[1][0][1] = 0;
    
    // 初始化主数组
    matrix[1][2] = 100;
    matrix[0][3] = 200;
    cube[1][2][0] = 300;
    cube[0][1][2] = 400;
    
    // 测试1: 二维数组的下标是二维数组访问
    // matrix[indices2d[0][0]][indices2d[0][1]] = matrix[1][2]
    int value1 = matrix[indices2d[0][0]][indices2d[0][1]];
    
    // 测试2: 三维数组的下标是二维数组访问
    // cube[indices2d[1][0]][indices2d[0][0]][indices2d[1][1]] = cube[0][1][3]
    int value2 = cube[indices2d[1][0]][indices2d[0][0]][indices2d[1][1]];
    
    // 测试3: 二维数组的下标是三维数组访问
    // matrix[indices3d[0][0][0]][indices3d[0][0][1]] = matrix[1][2]
    int value3 = matrix[indices3d[0][0][0]][indices3d[0][0][1]];
    
    // 测试4: 三维数组的下标是三维数组访问
    // cube[indices3d[0][1][0]][indices3d[0][1][1]][indices3d[1][0][1]] = cube[0][1][0]
    int value4 = cube[indices3d[0][1][0]][indices3d[0][1][1]][indices3d[1][0][1]];
    
    // 测试5: 更复杂的嵌套 - 下标表达式中包含运算
    // matrix[indices2d[0][0] + indices3d[1][0][0]][indices2d[0][1] - indices3d[0][0][1]]
    int value5 = matrix[indices2d[0][0] + indices3d[1][0][0]][indices2d[0][1] - indices3d[0][0][1]];
    
    // 测试6: 极限嵌套 - 多层多维数组索引
    // indices2d[indices3d[0][0][0]][indices3d[0][0][1]] 作为索引
    int idx1 = indices2d[indices3d[0][0][0]][indices3d[0][0][1]];
    int idx2 = indices2d[indices3d[0][1][0]][indices3d[0][1][1]];
    int value6 = matrix[idx1][idx2];
    
    // 返回所有值的和
    return value1 + value2 + value3 + value4 + value5 + value6;
}
