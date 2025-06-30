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

@__nested_array_0 = dso_local constant [1 x [3 x i32]] [[3 x i32] [i32 0, i32 0, i32 0]], align 16
@__nested_array_1 = dso_local constant [1 x [3 x i32]] [[3 x i32] [i32 0, i32 0, i32 0]], align 16
@__nested_array_2 = dso_local constant [1 x [3 x i32]] [[3 x i32] [i32 2, i32 1, i32 8]], align 16
@__nested_array_3 = dso_local constant [3 x i32] [i32 0, i32 0, i32 0], align 16
@__nested_array_4 = dso_local constant [1 x [3 x i32]] [[3 x i32] [i32 0, i32 0, i32 0]], align 16
@__const.main.c.0 = dso_local constant [4 x [1 x [3 x i32]]] [[1 x [3 x i32]] [[3 x i32] [i32 0, i32 0, i32 0]], [1 x [3 x i32]] [[3 x i32] [i32 0, i32 0, i32 0]], [1 x [3 x i32]] [[3 x i32] [i32 2, i32 1, i32 8]], [1 x [3 x i32]] [[3 x i32] [i32 0, i32 0, i32 0]]], align 16
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca [4 x [1 x [3 x i32]]], align 16 ; c
	%2 = bitcast [4 x [1 x [3 x i32]]]* %1 to i8*
	%3 = bitcast [4 x [1 x [3 x i32]]]* @__const.main.c.0 to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %2, i8* align 16 %3, i64 48, i1 false)
	%4 = getelementptr inbounds [4 x [1 x [3 x i32]]], [4 x [1 x [3 x i32]]]* %1, i64 0, i64 2
	%5 = getelementptr inbounds [1 x [3 x i32]], [1 x [3 x i32]]* %4, i64 0, i64 0
	%6 = getelementptr inbounds [3 x i32], [3 x i32]* %5, i64 0, i64 0
	%7 = load i32, i32* %6, align 4
	call void @putint(i32 noundef %7)
	%8 = getelementptr inbounds [4 x [1 x [3 x i32]]], [4 x [1 x [3 x i32]]]* %1, i64 0, i64 2
	%9 = getelementptr inbounds [1 x [3 x i32]], [1 x [3 x i32]]* %8, i64 0, i64 0
	%10 = getelementptr inbounds [3 x i32], [3 x i32]* %9, i64 0, i64 1
	%11 = load i32, i32* %10, align 4
	call void @putint(i32 noundef %11)
	%12 = getelementptr inbounds [4 x [1 x [3 x i32]]], [4 x [1 x [3 x i32]]]* %1, i64 0, i64 2
	%13 = getelementptr inbounds [1 x [3 x i32]], [1 x [3 x i32]]* %12, i64 0, i64 0
	%14 = getelementptr inbounds [3 x i32], [3 x i32]* %13, i64 0, i64 2
	%15 = load i32, i32* %14, align 4
	call void @putint(i32 noundef %15)
	call void @putch(i32 noundef 10)
	store i32 0, i32* %0, align 4
	br label %main_L1
main_L1:
	%16 = load i32, i32* %0, align 4
	ret i32 %16
}
