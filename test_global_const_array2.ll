; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

@__const.main.arr = dso_local constant [3 x i32] [i32 10, i32 20, i32 30], align 16
@arr = dso_local constant [3 x i32] [i32 10, i32 20, i32 30], align 16
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = getelementptr inbounds [3 x i32], [3 x i32]* @arr, i64 0, i64 0
	%2 = getelementptr inbounds [3 x i32], [3 x i32]* @arr, i64 0, i64 1
	%3 = load i32, i32* %1, align 4
	%4 = load i32, i32* %2, align 4
	%5 = add nsw i32 %3, %4
	%6 = getelementptr inbounds [3 x i32], [3 x i32]* @arr, i64 0, i64 2
	%7 = load i32, i32* %6, align 4
	%8 = add nsw i32 %5, %7
	store i32 %8, i32* %0, align 4
	br label %main_L1
main_L1:
	%9 = load i32, i32* %0, align 4
	ret i32 %9
}
