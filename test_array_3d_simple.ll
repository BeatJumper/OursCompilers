; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

@__nested_array_0 = dso_local constant [2 x i32] [i32 1, i32 2], align 16
@__nested_array_1 = dso_local constant [2 x i32] [i32 3, i32 4], align 16
@__nested_array_2 = dso_local constant [0 x i32] zeroinitializer, align 16
@__const.main.arr.0 = dso_local constant [4 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4], [2 x i32] [i32 0, i32 0], [2 x i32] [i32 7, i32 0]], align 16
@__const.main.c = dso_local constant [4 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4], [2 x i32] [i32 5, i32 6], [2 x i32] [i32 7, i32 8]], align 16
@__nested_array_3 = dso_local constant [1 x i32] [i32 1], align 16
@__nested_array_4 = dso_local constant [1 x i32] [i32 2], align 16
@__nested_array_5 = dso_local constant [2 x [1 x i32]] [[1 x i32] [i32 1], [1 x i32] [i32 2]], align 16
@__nested_array_6 = dso_local constant [1 x i32] [i32 3], align 16
@__nested_array_7 = dso_local constant [1 x i32] [i32 4], align 16
@__nested_array_8 = dso_local constant [2 x [1 x i32]] [[1 x i32] [i32 3], [1 x i32] [i32 4]], align 16
@__nested_array_9 = dso_local constant [1 x i32] [i32 5], align 16
@__nested_array_10 = dso_local constant [1 x i32] [i32 6], align 16
@__nested_array_11 = dso_local constant [2 x [1 x i32]] [[1 x i32] [i32 5], [1 x i32] [i32 6]], align 16
@__nested_array_12 = dso_local constant [1 x i32] [i32 7], align 16
@__nested_array_13 = dso_local constant [1 x i32] [i32 8], align 16
@__nested_array_14 = dso_local constant [2 x [1 x i32]] [[1 x i32] [i32 7], [1 x i32] [i32 8]], align 16
@__const.main.e = dso_local constant [4 x [2 x [1 x i32]]] [[2 x [1 x i32]] [[1 x i32] [i32 1], [1 x i32] [i32 2]], [2 x [1 x i32]] [[1 x i32] [i32 3], [1 x i32] [i32 4]], [2 x [1 x i32]] [[1 x i32] [i32 5], [1 x i32] [i32 6]], [2 x [1 x i32]] [[1 x i32] [i32 7], [1 x i32] [i32 8]]], align 16
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = load [4 x [2 x i32]], [4 x [2 x i32]]* @__const.main.arr.0, align 4
	%2 = alloca [4 x [2 x i32]], align 4 ; a
	store [4 x [2 x i32]] %1, [4 x [2 x i32]]* %2, align 4
	%3 = alloca [4 x [2 x i32]], align 16 ; b
	%4 = bitcast [4 x [2 x i32]]* %3 to i8*
	call void @llvm.memset.p0i8.i64(i8* align 16 %4, i8 0, i64 32, i1 false)
	%5 = alloca [4 x [2 x i32]], align 16 ; c
	%6 = bitcast [4 x [2 x i32]]* %5 to i8*
	%7 = bitcast [4 x [2 x i32]]* @__const.main.c to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %6, i8* align 16 %7, i64 32, i1 false)
	%8 = alloca [4 x [2 x i32]], align 16 ; d_ACTUAL_SIZE_32
	%9 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 0
	%10 = getelementptr inbounds [2 x i32], [2 x i32]* %9, i64 0, i64 0
	store i32 1, i32* %10, align 4
	%11 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 0
	%12 = getelementptr inbounds [2 x i32], [2 x i32]* %11, i64 0, i64 1
	store i32 2, i32* %12, align 4
	%13 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 1
	%14 = getelementptr inbounds [2 x i32], [2 x i32]* %13, i64 0, i64 0
	store i32 3, i32* %14, align 4
	%15 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 1
	%16 = getelementptr inbounds [2 x i32], [2 x i32]* %15, i64 0, i64 1
	store i32 0, i32* %16, align 4
	%17 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 2
	%18 = getelementptr inbounds [2 x i32], [2 x i32]* %17, i64 0, i64 0
	store i32 5, i32* %18, align 4
	%19 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 2
	%20 = getelementptr inbounds [2 x i32], [2 x i32]* %19, i64 0, i64 1
	store i32 0, i32* %20, align 4
	%21 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 3
	%22 = getelementptr inbounds [2 x i32], [2 x i32]* %21, i64 0, i64 0
	%23 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %2, i64 0, i64 3
	%24 = getelementptr inbounds [2 x i32], [2 x i32]* %23, i64 0, i64 0
	%25 = load i32, i32* %24, align 4
	store i32 %25, i32* %22, align 4
	%26 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 3
	%27 = getelementptr inbounds [2 x i32], [2 x i32]* %26, i64 0, i64 1
	store i32 8, i32* %27, align 4
	%28 = alloca [4 x [2 x [1 x i32]]], align 16 ; e
	%29 = bitcast [4 x [2 x [1 x i32]]]* %28 to i8*
	%30 = bitcast [4 x [2 x [1 x i32]]]* @__const.main.e to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %29, i8* align 16 %30, i64 32, i1 false)
	%31 = getelementptr inbounds [4 x [2 x [1 x i32]]], [4 x [2 x [1 x i32]]]* %28, i64 0, i64 3
	%32 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %31, i64 0, i64 1
	%33 = getelementptr inbounds [1 x i32], [1 x i32]* %32, i64 0, i64 0
	%34 = getelementptr inbounds [4 x [2 x [1 x i32]]], [4 x [2 x [1 x i32]]]* %28, i64 0, i64 0
	%35 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %34, i64 0, i64 0
	%36 = getelementptr inbounds [1 x i32], [1 x i32]* %35, i64 0, i64 0
	%37 = load i32, i32* %33, align 4
	%38 = load i32, i32* %36, align 4
	%39 = add nsw i32 %37, %38
	%40 = getelementptr inbounds [4 x [2 x [1 x i32]]], [4 x [2 x [1 x i32]]]* %28, i64 0, i64 0
	%41 = getelementptr inbounds [2 x [1 x i32]], [2 x [1 x i32]]* %40, i64 0, i64 1
	%42 = getelementptr inbounds [1 x i32], [1 x i32]* %41, i64 0, i64 0
	%43 = load i32, i32* %42, align 4
	%44 = add nsw i32 %39, %43
	%45 = getelementptr inbounds [4 x [2 x i32]], [4 x [2 x i32]]* %8, i64 0, i64 3
	%46 = getelementptr inbounds [2 x i32], [2 x i32]* %45, i64 0, i64 0
	%47 = load i32, i32* %46, align 4
	%48 = add nsw i32 %44, %47
	store i32 %48, i32* %0, align 4
	br label %main_L1
main_L1:
	%49 = load i32, i32* %0, align 4
	ret i32 %49
}
