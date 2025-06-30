#include <stdio.h>

int main() {
    // 2MB array - same as AddFunction
    int big_array[2][2][2][2][2][2][2][2][2][2][2][2][2][2][2][2][2][2][2];
    
    // Initialize to zero
    for(int i = 0; i < 2; i++) {
        // Just access first element to prevent optimization
        big_array[i][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0] = i;
    }
    
    printf("Test completed: %d\n", big_array[1][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0]);
    return 0;
}
