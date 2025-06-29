; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

@__const.main.arr = dso_local constant [1 x i32] [i32 42], align 16
@__nested_array_0 = dso_local constant [1 x i32] [i32 99], align 16
@__const.main.nested = dso_local constant [1 x [1 x i32]] [[1 x i32] [i32 99]], align 16
@__const.main.c = dso_local constant [2 x [1 x i32]] [[1 x i32] [i32 6], [1 x i32] [i32 7]], align 16
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca [1 x i32], align 16 ; arr
	%2 = bitcast [1 x i32]* %1 to i8*
	%3 = bitcast [1 x i32]* @__const.main.arr to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %2, i8* align 16 %3, i64 4, i1 false)
	%4 = alloca i32, align 4 ; x
	%5 = getelementptr inbounds [1 x i32], [1 x i32]* %1, i64 0, i64 0
	%6 = load i32, i32* %5, align 4
	store i32 %6, i32* %4, align 4
	%7 = alloca [1 x [1 x i32]], align 16 ; nested
	%8 = bitcast [1 x [1 x i32]]* %7 to i8*
	%9 = bitcast [1 x [1 x i32]]* @__const.main.nested to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %8, i8* align 16 %9, i64 4, i1 false)
	%10 = alloca i32, align 4 ; y
	%11 = getelementptr inbounds [1 x [1 x i32]], [1 x [1 x i32]]* %7, i64 0, i64 0
	%12 = getelementptr inbounds [1 x i32], [1 x i32]* %11, i64 0, i64 0
	%13 = load i32, i32* %12, align 4
	store i32 %13, i32* %10, align 4
	%14 = alloca [2 x [1 x i32]], align 16 ; c
	%15 = bitcast [2 x [1 x i32]]* %14 to i8*
	%16 = bitcast [2 x [1 x i32]]* @__const.main.c to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %15, i8* align 16 %16, i64 8, i1 false)
	%17 = alloca [2 x [1 x i32]], align 16 ; result
	%18 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %17, i64 0, i64 0
	%19 = getelementptr inbounds [1 x i32], [1 x i32]* %18, i64 0, i64 0
	%20 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %14, i64 0, i64 0
	%21 = getelementptr inbounds [1 x i32], [1 x i32]* %20, i64 0, i64 0
	%22 = load i32, i32* %21, align 4
	store i32 %22, i32* %19, align 4
	%23 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %17, i64 0, i64 1
	%24 = getelementptr inbounds [1 x i32], [1 x i32]* %23, i64 0, i64 0
	%25 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %14, i64 0, i64 1
	%26 = getelementptr inbounds [1 x i32], [1 x i32]* %25, i64 0, i64 0
	%27 = load i32, i32* %26, align 4
	store i32 %27, i32* %24, align 4
	%28 = load i32, i32* %4, align 4
	%29 = load i32, i32* %10, align 4
	%30 = add nsw i32 %28, %29
	%31 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %17, i64 0, i64 0
	%32 = getelementptr inbounds [1 x i32], [1 x i32]* %31, i64 0, i64 0
	%33 = load i32, i32* %32, align 4
	%34 = add nsw i32 %30, %33
	%35 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %17, i64 0, i64 1
	%36 = getelementptr inbounds [1 x i32], [1 x i32]* %35, i64 0, i64 0
	%37 = load i32, i32* %36, align 4
	%38 = add nsw i32 %34, %37
	store i32 %38, i32* %0, align 4
	br label %main_L1
main_L1:
	%39 = load i32, i32* %0, align 4
	ret i32 %39
}
