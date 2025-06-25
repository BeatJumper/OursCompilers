// 测试多维数组的复杂嵌套访问
int main() {
    // 声明多维数组
    int matrix[3][4];
    int cube[2][3][4];
    int indices[2];
    
    // 初始化索引数组
    indices[0] = 1;
    indices[1] = 2;
    
    // 基本赋值
    matrix[1][2] = 42;
    cube[1][2][3] = 100;
    
    // 使用变量作为索引的嵌套访问
    int i = 1;
    int j = 2;
    int k = 3;
    
    // 变量索引访问
    matrix[i][j] = 50;
    cube[i][j][k] = 200;
    
    // 使用数组元素作为索引的嵌套访问
    matrix[indices[0]][indices[1]] = 60;
    cube[indices[0]][indices[1]][k] = 300;
    
    // 复杂的嵌套表达式作为索引
    matrix[indices[0] + 1][indices[1] - 1] = 70;
    cube[i - 1][j + 1][k - 1] = 400;
    
    // 多层嵌套：数组访问结果作为另一个数组的索引
    int coord[2];
    coord[0] = 1;
    coord[1] = 2;
    
    // 使用一个数组的元素作为另一个数组的索引
    int value1 = matrix[coord[0]][coord[1]];
    int value2 = cube[coord[0]][coord[1]][indices[0]];
    
    // 更复杂的嵌套：数组访问链
    int result = matrix[indices[coord[0]]][indices[coord[1]]];
    
    // 混合运算中的嵌套访问
    int final_result = matrix[i][j] + cube[coord[0]][coord[1]][k] + result;
    
    return final_result;
}
