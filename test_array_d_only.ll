; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

@__const.main.arr.0 = dso_local constant [4 x [2 x i32]] [[2 x i32] [i32 1, i32 0], [2 x i32] [i32 2, i32 0], [2 x i32] [i32 3, i32 0], [2 x i32] [i32 4, i32 0]], align 16
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = load [4 x [2 x i32]], [4 x [2 x i32]]* @__const.main.arr.0, align 4
	%2 = alloca [4 x [2 x i32]], align 4 ; a
	store [4 x [2 x i32]] %1, [4 x [2 x i32]]* %2, align 4
	%3 = alloca [4 x [2 x i32]], align 16 ; d_ACTUAL_SIZE_32
	%4 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 0
	%5 = getelementptr inbounds [2 x i32], [2 x i32]* %4, i64 0, i64 0
	store i32 1, i32* %5, align 4
	%6 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 0
	%7 = getelementptr inbounds [2 x i32], [2 x i32]* %6, i64 0, i64 1
	store i32 2, i32* %7, align 4
	%8 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 1
	%9 = getelementptr inbounds [2 x i32], [2 x i32]* %8, i64 0, i64 0
	store i32 3, i32* %9, align 4
	%10 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 1
	%11 = getelementptr inbounds [2 x i32], [2 x i32]* %10, i64 0, i64 1
	store i32 0, i32* %11, align 4
	%12 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 2
	%13 = getelementptr inbounds [2 x i32], [2 x i32]* %12, i64 0, i64 0
	store i32 5, i32* %13, align 4
	%14 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 2
	%15 = getelementptr inbounds [2 x i32], [2 x i32]* %14, i64 0, i64 1
	store i32 0, i32* %15, align 4
	%16 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 3
	%17 = getelementptr inbounds [2 x i32], [2 x i32]* %16, i64 0, i64 0
	%18 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %2, i64 0, i64 3
	%19 = getelementptr inbounds [2 x i32], [2 x i32]* %18, i64 0, i64 0
	%20 = load i32, i32* %19, align 4
	store i32 %20, i32* %17, align 4
	%21 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 3
	%22 = getelementptr inbounds [2 x i32], [2 x i32]* %21, i64 0, i64 1
	store i32 8, i32* %22, align 4
	%23 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %3, i64 0, i64 3
	%24 = getelementptr inbounds [2 x i32], [2 x i32]* %23, i64 0, i64 0
	%25 = load i32, i32* %24, align 8
	store i32 %25, i32* %0, align 4
	br label %main_L1
main_L1:
	%26 = load i32, i32* %0, align 4
	ret i32 %26
}
