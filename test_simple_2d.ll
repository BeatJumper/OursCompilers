; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

@__nested_array_0 = dso_local constant [2 x i32] [i32 1, i32 2], align 16
@__nested_array_1 = dso_local constant [2 x i32] [i32 3, i32 4], align 16
@__const.main.arr.0 = dso_local constant [2 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4]], align 16
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = load [2 x [2 x i32]], [2 x [2 x i32]]* @__const.main.arr.0, align 4
	%2 = alloca [2 x [2 x i32]], align 4 ; a
	store [2 x [2 x i32]] %1, [2 x [2 x i32]]* %2, align 4
	%3 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %2, i64 0, i64 1
	%4 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 1
	%5 = load i32, i32* %4, align 8
	store i32 %5, i32* %0, align 4
	br label %main_L1
main_L1:
	%6 = load i32, i32* %0, align 4
	ret i32 %6
}
