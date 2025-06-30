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

@__const.main.a = dso_local constant [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5], align 16
define dso_local i32 @test_func(i32* noundef %0) #0 {
test_func_L0:
	%1 = alloca i32*, align 8 ; arr
	store i32* %0, i32** %1, align 8
	%2 = alloca i32, align 4 ; __ret
	%3 = load i32*, i32** %1, align 8
	%4 = getelementptr inbounds i32, i32* %3, i64 0
	%5 = load i32, i32* %4, align 8
	store i32 %5, i32* %2, align 4
	br label %test_func_L1
test_func_L1:
	%6 = load i32, i32* %2, align 4
	ret i32 %6
}
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca [5 x i32], align 16 ; a
	%2 = bitcast [5 x i32]* %1 to i8*
	%3 = bitcast [5 x i32]* @__const.main.a to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %2, i8* align 16 %3, i64 20, i1 false)
	%4 = getelementptr inbounds [5 x i32], [5 x i32]* %1, i64 0, i64 0
	%5 = call i32 @test_func(i32* noundef %4)
	store i32 %5, i32* %0, align 4
	br label %main_L1
main_L1:
	%6 = load i32, i32* %0, align 4
	ret i32 %6
}
