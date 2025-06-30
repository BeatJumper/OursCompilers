; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

; Standard library function declarations
declare i32 @getint()
declare i32 @getch()
declare float @getfloat()
declare i32 @getarray(i32*)
declare i32 @getfarray(float*)
declare void @putint(i32)
declare void @putch(i32)
declare void @putfloat(float)
declare void @putarray(i32, i32*)
declare void @putfarray(i32, float*)
declare void @putf(i8*)
declare void @_sysy_starttime(i32)
declare void @_sysy_stoptime(i32)

@__nested_array_0 = dso_local constant [3 x i32] [i32 1, i32 2, i32 3], align 16
@__nested_array_1 = dso_local constant [3 x i32] [i32 4, i32 5, i32 6], align 16
@__const.main.matrix.0 = dso_local constant [2 x [3 x i32]] [[3 x i32] [i32 1, i32 2, i32 3], [3 x i32] [i32 4, i32 5, i32 6]], align 16
@__nested_array_2 = dso_local constant [3 x i32] [i32 1, i32 2, i32 3], align 16
@__nested_array_3 = dso_local constant [3 x i32] [i32 4, i32 5, i32 6], align 16
@__nested_array_4 = dso_local constant [2 x [3 x i32]] [[3 x i32] [i32 1, i32 2, i32 3], [3 x i32] [i32 4, i32 5, i32 6]], align 16
@__nested_array_5 = dso_local constant [3 x i32] [i32 7, i32 8, i32 9], align 16
@__nested_array_6 = dso_local constant [3 x i32] [i32 10, i32 11, i32 12], align 16
@__nested_array_7 = dso_local constant [2 x [3 x i32]] [[3 x i32] [i32 7, i32 8, i32 9], [3 x i32] [i32 10, i32 11, i32 12]], align 16
@__const.main.cube.1 = dso_local constant [2 x [2 x [3 x i32]]] [[2 x [3 x i32]] [[3 x i32] [i32 1, i32 2, i32 3], [3 x i32] [i32 4, i32 5, i32 6]], [2 x [3 x i32]] [[3 x i32] [i32 7, i32 8, i32 9], [3 x i32] [i32 10, i32 11, i32 12]]], align 16
define dso_local i32 @sum_2d([3 x i32]* noundef %0, i32 noundef %1) #0 {
sum_2d_L0:
	%2 = alloca [3 x i32]*, align 8 ; arr
	%3 = alloca i32, align 4 ; rows
	store [3 x i32]* %0, [3 x i32]** %2, align 8
	store i32 %1, i32* %3, align 8
	%4 = alloca i32, align 4 ; __ret
	%5 = alloca i32, align 4 ; total
	%6 = alloca i32, align 4 ; i
	%7 = alloca i32, align 4 ; j
	store i32 0, i32* %5, align 4
	store i32 0, i32* %6, align 4
	br label %sum_2d_L1
sum_2d_L1:
	%8 = load i32, i32* %6, align 4
	%9 = load i32, i32* %3, align 4
	%10 = icmp slt i32 %8, %9
	br i1 %10, label %sum_2d_L2, label %sum_2d_L6
sum_2d_L2:
	store i32 0, i32* %7, align 4
	br label %sum_2d_L3
sum_2d_L3:
	%11 = load i32, i32* %7, align 4
	%12 = icmp slt i32 %11, 3
	br i1 %12, label %sum_2d_L4, label %sum_2d_L5
sum_2d_L4:
	%13 = load [3 x i32]*, [3 x i32]** %2, align 8
	%14 = load i32, i32* %6, align 4
	%15 = sext i32 %14 to i64
	%16 = getelementptr inbounds [3 x i32], [3 x i32]* %13, i64 %15
	%17 = load i32, i32* %7, align 4
	%18 = sext i32 %17 to i64
	%19 = getelementptr inbounds [3 x i32], [3 x i32]* %16, i64 0, i64 %18
	%20 = load i32, i32* %5, align 4
	%21 = load i32, i32* %19, align 4
	%22 = add nsw i32 %20, %21
	store i32 %22, i32* %5, align 4
	%23 = load i32, i32* %7, align 4
	%24 = add nsw i32 %23, 1
	store i32 %24, i32* %7, align 4
	br label %sum_2d_L3
sum_2d_L5:
	%25 = load i32, i32* %6, align 4
	%26 = add nsw i32 %25, 1
	store i32 %26, i32* %6, align 4
	br label %sum_2d_L1
sum_2d_L6:
	%27 = load i32, i32* %5, align 8
	store i32 %27, i32* %4, align 4
	br label %sum_2d_L7
sum_2d_L7:
	%28 = load i32, i32* %4, align 4
	ret i32 %28
}
define dso_local i32 @sum_3d([2 x [3 x i32]]* noundef %0, i32 noundef %1) #0 {
sum_3d_L0:
	%2 = alloca [2 x [3 x i32]]*, align 8 ; arr
	%3 = alloca i32, align 4 ; layers
	store [2 x [3 x i32]]* %0, [2 x [3 x i32]]** %2, align 8
	store i32 %1, i32* %3, align 8
	%4 = alloca i32, align 4 ; __ret
	%5 = alloca i32, align 4 ; total
	%6 = alloca i32, align 4 ; i
	%7 = alloca i32, align 4 ; j
	%8 = alloca i32, align 4 ; k
	store i32 0, i32* %5, align 4
	store i32 0, i32* %6, align 4
	br label %sum_3d_L1
sum_3d_L1:
	%9 = load i32, i32* %6, align 4
	%10 = load i32, i32* %3, align 4
	%11 = icmp slt i32 %9, %10
	br i1 %11, label %sum_3d_L2, label %sum_3d_L9
sum_3d_L2:
	store i32 0, i32* %7, align 4
	br label %sum_3d_L3
sum_3d_L3:
	%12 = load i32, i32* %7, align 4
	%13 = icmp slt i32 %12, 2
	br i1 %13, label %sum_3d_L4, label %sum_3d_L8
sum_3d_L4:
	store i32 0, i32* %8, align 4
	br label %sum_3d_L5
sum_3d_L5:
	%14 = load i32, i32* %8, align 4
	%15 = icmp slt i32 %14, 3
	br i1 %15, label %sum_3d_L6, label %sum_3d_L7
sum_3d_L6:
	%16 = load [2 x [3 x i32]]*, [2 x [3 x i32]]** %2, align 8
	%17 = load i32, i32* %6, align 4
	%18 = sext i32 %17 to i64
	%19 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %16, i64 %18
	%20 = load i32, i32* %7, align 4
	%21 = sext i32 %20 to i64
	%22 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %19, i64 0, i64 %21
	%23 = load i32, i32* %8, align 4
	%24 = sext i32 %23 to i64
	%25 = getelementptr inbounds [3 x i32], [3 x i32]* %22, i64 0, i64 %24
	%26 = load i32, i32* %5, align 4
	%27 = load i32, i32* %25, align 4
	%28 = add nsw i32 %26, %27
	store i32 %28, i32* %5, align 4
	%29 = load i32, i32* %8, align 4
	%30 = add nsw i32 %29, 1
	store i32 %30, i32* %8, align 4
	br label %sum_3d_L5
sum_3d_L7:
	%31 = load i32, i32* %7, align 4
	%32 = add nsw i32 %31, 1
	store i32 %32, i32* %7, align 4
	br label %sum_3d_L3
sum_3d_L8:
	%33 = load i32, i32* %6, align 4
	%34 = add nsw i32 %33, 1
	store i32 %34, i32* %6, align 4
	br label %sum_3d_L1
sum_3d_L9:
	%35 = load i32, i32* %5, align 8
	store i32 %35, i32* %4, align 4
	br label %sum_3d_L10
sum_3d_L10:
	%36 = load i32, i32* %4, align 4
	ret i32 %36
}
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca [2 x [3 x i32]], align 16 ; matrix
	%2 = alloca [2 x [2 x [3 x i32]]], align 16 ; cube
	%3 = alloca i32, align 4 ; result2d
	%4 = alloca i32, align 4 ; result3d
	%5 = bitcast [2 x [3 x i32]]* %1 to i8*
	%6 = bitcast [2 x [3 x i32]]* @__const.main.matrix.0 to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %5, i8* align 16 %6, i64 24, i1 false)
	%7 = bitcast [2 x [2 x [3 x i32]]]* %2 to i8*
	%8 = bitcast [2 x [2 x [3 x i32]]]* @__const.main.cube.1 to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %7, i8* align 16 %8, i64 48, i1 false)
	%9 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 0
	%10 = call i32 @sum_2d([3 x i32]* noundef %9, i32 noundef 2)
	store i32 %10, i32* %3, align 4
	%11 = load i32, i32* %3, align 4
	call void @putint(i32 noundef %11)
	call void @putch(i32 noundef 10)
	%12 = getelementptr inbounds [2 x [2 x [3 x i32]]], [2 x [2 x [3 x i32]]]* %2, i64 0, i64 0
	%13 = call i32 @sum_3d([2 x [3 x i32]]* noundef %12, i32 noundef 2)
	store i32 %13, i32* %4, align 4
	%14 = load i32, i32* %4, align 4
	call void @putint(i32 noundef %14)
	call void @putch(i32 noundef 10)
	store i32 0, i32* %0, align 4
	br label %main_L1
main_L1:
	%15 = load i32, i32* %0, align 4
	ret i32 %15
}
