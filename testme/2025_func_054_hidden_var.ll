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

@b = private unnamed_addr global i32 5, align 4
@c = private unnamed_addr global [4 x i32] [i32 6, i32 7, i32 8, i32 9], align 4
@__nested_array_0 = dso_local constant [8 x i32] [i32 0, i32 9, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0], align 16
@__const.main.c.0 = dso_local constant [2 x [8 x i32]] [[8 x i32] [i32 0, i32 9, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0], [8 x i32] [i32 8, i32 3, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0]], align 16
@__nested_array_1 = dso_local constant [1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]], align 16
@__nested_array_2 = dso_local constant [1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]], align 16
@__nested_array_3 = dso_local constant [5 x i32] [i32 2, i32 1, i32 8, i32 0, i32 0], align 16
@__nested_array_4 = dso_local constant [5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0], align 16
@__nested_array_5 = dso_local constant [1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]], align 16
@__const.main.c.1 = dso_local constant [7 x [1 x [5 x i32]]] [[1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]], [1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]], [1 x [5 x i32]] [[5 x i32] [i32 2, i32 1, i32 8, i32 0, i32 0]], [1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]], [1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]], [1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]], [1 x [5 x i32]] [[5 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0]]], align 16
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca i32, align 4 ; a
	store i32 1, i32* %1, align 4
	%2 = alloca i32, align 4 ; a
	store i32 2, i32* %2, align 4
	store i32 3, i32* %2, align 4
	%3 = load i32, i32* %2, align 4
	call void @putint(i32 noundef %3)
	%4 = load i32, i32* %2, align 4
	call void @putint(i32 noundef %4)
	%5 = load i32, i32* %1, align 4
	call void @putint(i32 noundef %5)
	call void @putch(i32 noundef 10)
	br label %main_L1
main_L1:
	%6 = load i32, i32* %1, align 4
	%7 = icmp slt i32 %6, 5
	br i1 %7, label %main_L2, label %main_L5
main_L2:
	%8 = alloca i32, align 4 ; a
	store i32 0, i32* %8, align 4
	%9 = load i32, i32* %8, align 4
	%10 = add nsw i32 %9, 1
	store i32 %10, i32* %8, align 4
	%11 = load i32, i32* %8, align 4
	%12 = icmp ne i32 %11, 0
	br i1 %12, label %main_L3, label %main_L4
main_L3:
	br label %main_L5
main_L4:
	br label %main_L1
main_L5:
	%13 = load i32, i32* %1, align 4
	call void @putint(i32 noundef %13)
	call void @putch(i32 noundef 10)
	%14 = getelementptr inbounds [4 x i32], [4 x i32]* @c, i64 0, i64 2
	store i32 1, i32* %14, align 4
	%15 = alloca [2 x [8 x i32]], align 16 ; c
	%16 = bitcast [2 x [8 x i32]]* %15 to i8*
	%17 = bitcast [2 x [8 x i32]]* @__const.main.c.0 to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %16, i8* align 16 %17, i64 64, i1 false)
	%18 = alloca i32, align 4 ; b
	store i32 2, i32* %18, align 4
	%19 = getelementptr inbounds [4 x i32], [4 x i32]* @c, i64 0, i64 2
	%20 = load i32, i32* %19, align 4
	%21 = icmp ne i32 %20, 0
	br i1 %21, label %main_L6, label %main_L7
main_L6:
	%22 = alloca [7 x [1 x [5 x i32]]], align 16 ; c
	%23 = bitcast [7 x [1 x [5 x i32]]]* %22 to i8*
	%24 = bitcast [7 x [1 x [5 x i32]]]* @__const.main.c.1 to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %23, i8* align 16 %24, i64 140, i1 false)
	%25 = load i32, i32* %18, align 4
	%26 = sext i32 %25 to i64
	%27 = getelementptr inbounds [7 x [1 x [5 x i32]]], [7 x [1 x [5 x i32]]]* %22, i64 0, i64 %26
	%28 = getelementptr inbounds [1 x [5 x i32]], [1 x [5 x i32]]* %27, i64 0, i64 0
	%29 = getelementptr inbounds [5 x i32], [5 x i32]* %28, i64 0, i64 0
	%30 = load i32, i32* %29, align 4
	call void @putint(i32 noundef %30)
	%31 = load i32, i32* %18, align 4
	%32 = sext i32 %31 to i64
	%33 = getelementptr inbounds [7 x [1 x [5 x i32]]], [7 x [1 x [5 x i32]]]* %22, i64 0, i64 %32
	%34 = getelementptr inbounds [1 x [5 x i32]], [1 x [5 x i32]]* %33, i64 0, i64 0
	%35 = getelementptr inbounds [5 x i32], [5 x i32]* %34, i64 0, i64 1
	%36 = load i32, i32* %35, align 4
	call void @putint(i32 noundef %36)
	%37 = load i32, i32* %18, align 4
	%38 = sext i32 %37 to i64
	%39 = getelementptr inbounds [7 x [1 x [5 x i32]]], [7 x [1 x [5 x i32]]]* %22, i64 0, i64 %38
	%40 = getelementptr inbounds [1 x [5 x i32]], [1 x [5 x i32]]* %39, i64 0, i64 0
	%41 = getelementptr inbounds [5 x i32], [5 x i32]* %40, i64 0, i64 2
	%42 = load i32, i32* %41, align 4
	call void @putint(i32 noundef %42)
	br label %main_L7
main_L7:
	call void @putch(i32 noundef 10)
	%43 = load i32, i32* @b, align 4
	call void @putint(i32 noundef %43)
	call void @putch(i32 noundef 10)
	%44 = getelementptr inbounds [4 x i32], [4 x i32]* @c, i64 0, i64 0
	%45 = load i32, i32* %44, align 4
	call void @putint(i32 noundef %45)
	%46 = getelementptr inbounds [4 x i32], [4 x i32]* @c, i64 0, i64 1
	%47 = load i32, i32* %46, align 4
	call void @putint(i32 noundef %47)
	%48 = getelementptr inbounds [4 x i32], [4 x i32]* @c, i64 0, i64 2
	%49 = load i32, i32* %48, align 4
	call void @putint(i32 noundef %49)
	%50 = getelementptr inbounds [4 x i32], [4 x i32]* @c, i64 0, i64 3
	%51 = load i32, i32* %50, align 4
	call void @putint(i32 noundef %51)
	call void @putch(i32 noundef 10)
	call void @putint(i32 noundef 0)
	call void @putch(i32 noundef 10)
	store i32 0, i32* %0, align 4
	br label %main_L8
main_L8:
	%52 = load i32, i32* %0, align 4
	ret i32 %52
}
