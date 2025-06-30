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

@arr = private unnamed_addr global [2 x [3 x i32]] [[3 x i32] [i32 1, i32 2, i32 3], [3 x i32] [i32 4, i32 5, i32 6]], align 4
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* @arr, i64 0, i64 0
	%2 = getelementptr inbounds [3 x i32], [3 x i32]* %1, i64 0, i64 0
	%3 = load i32, i32* %2, align 4
	call void @putint(i32 noundef %3)
	%4 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* @arr, i64 0, i64 0
	%5 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 1
	%6 = load i32, i32* %5, align 4
	call void @putint(i32 noundef %6)
	%7 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* @arr, i64 0, i64 1
	%8 = getelementptr inbounds [3 x i32], [3 x i32]* %7, i64 0, i64 0
	%9 = load i32, i32* %8, align 4
	call void @putint(i32 noundef %9)
	%10 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* @arr, i64 0, i64 1
	%11 = getelementptr inbounds [3 x i32], [3 x i32]* %10, i64 0, i64 1
	%12 = load i32, i32* %11, align 4
	call void @putint(i32 noundef %12)
	store i32 0, i32* %0, align 4
	br label %main_L1
main_L1:
	%13 = load i32, i32* %0, align 4
	ret i32 %13
}
