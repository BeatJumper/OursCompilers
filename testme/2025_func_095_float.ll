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

@RADIUS = private unnamed_addr constant float 0x4016000000000000, align 4
@PI = private unnamed_addr constant float 0x400921fb60000000, align 4
@EPS = private unnamed_addr constant float 0x3eb0c6f7a0000000, align 4
@PI_HEX = private unnamed_addr constant float 0x400921fb60000000, align 4
@HEX2 = private unnamed_addr constant float 0x3fb4000000000000, align 4
@FACT = private unnamed_addr constant float 0xc0e01d0000000000, align 4
@EVAL1 = private unnamed_addr constant float 0x4057c21fc0000000, align 4
@EVAL2 = private unnamed_addr constant float 0x4041475ce0000000, align 4
@EVAL3 = private unnamed_addr constant float 0x4041475ce0000000, align 4
@CONV1 = private unnamed_addr constant float 0x406d200000000000, align 4
@CONV2 = private unnamed_addr constant float 0x40affe0000000000, align 4
@MAX = private unnamed_addr constant i32 1000000000, align 4
@TWO = private unnamed_addr constant i32 2, align 4
@THREE = private unnamed_addr constant i32 3, align 4
@FIVE = private unnamed_addr constant i32 5, align 4
@__const.main.arr.0 = dso_local constant [10 x float] [float 0x3ff0000000000000, float 0x4000000000000000, float 0x0000000000000000, float 0x0000000000000000, float 0x0000000000000000, float 0x0000000000000000, float 0x0000000000000000, float 0x0000000000000000, float 0x0000000000000000, float 0x0000000000000000], align 16
define dso_local float @float_abs(float noundef %0) #0 {
float_abs_L0:
	%1 = alloca float, align 4 ; x
	store float %0, float* %1, align 8
	%2 = alloca float, align 4 ; __ret
	%3 = load float, float* %1, align 4
	%4 = fcmp olt float %3, 0x0000000000000000
	br i1 %4, label %float_abs_L1, label %float_abs_L2
float_abs_L1:
	%5 = load float, float* %1, align 4
	%6 = fsub float 0x0000000000000000, %5
	store float %6, float* %2, align 4
	br label %float_abs_L3
float_abs_L2:
	%7 = load float, float* %1, align 8
	store float %7, float* %2, align 4
	br label %float_abs_L3
float_abs_L3:
	%8 = load float, float* %2, align 4
	ret float %8
}
define dso_local float @circle_area(i32 noundef %0) #0 {
circle_area_L0:
	%1 = alloca i32, align 4 ; radius
	store i32 %0, i32* %1, align 8
	%2 = alloca float, align 4 ; __ret
	%3 = load float, float* @PI, align 4
	%4 = load i32, i32* %1, align 4
	%5 = sitofp i32 %4 to float
	%6 = fmul float %3, %5
	%7 = load i32, i32* %1, align 4
	%8 = sitofp i32 %7 to float
	%9 = fmul float %6, %8
	%10 = load i32, i32* %1, align 4
	%11 = load i32, i32* %1, align 4
	%12 = mul nsw i32 %10, %11
	%13 = load float, float* @PI, align 4
	%14 = sitofp i32 %12 to float
	%15 = fmul float %14, %13
	%16 = fadd float %9, %15
	%17 = sitofp i32 2 to float
	%18 = fdiv float %16, %17
	store float %18, float* %2, align 4
	br label %circle_area_L1
circle_area_L1:
	%19 = load float, float* %2, align 4
	ret float %19
}
define dso_local i32 @float_eq(float noundef %0, float noundef %1) #0 {
float_eq_L0:
	%2 = alloca float, align 4 ; a
	%3 = alloca float, align 4 ; b
	store float %0, float* %2, align 8
	store float %1, float* %3, align 8
	%4 = alloca i32, align 4 ; __ret
	%5 = load float, float* %2, align 4
	%6 = load float, float* %3, align 4
	%7 = fsub float %5, %6
	%8 = call float @float_abs(float noundef %7)
	%9 = load float, float* @EPS, align 4
	%10 = fcmp olt float %8, %9
	br i1 %10, label %float_eq_L1, label %float_eq_L2
float_eq_L1:
	%11 = fptosi float 0x3ff0000000000000 to i32
	store i32 %11, i32* %4, align 4
	br label %float_eq_L3
float_eq_L2:
	store i32 0, i32* %4, align 4
	br label %float_eq_L3
float_eq_L3:
	%12 = load i32, i32* %4, align 4
	ret i32 %12
}
define dso_local void @error() #0 {
error_L0:
	call void @putch(i32 noundef 101)
	call void @putch(i32 noundef 114)
	call void @putch(i32 noundef 114)
	call void @putch(i32 noundef 111)
	call void @putch(i32 noundef 114)
	call void @putch(i32 noundef 10)
	br label %error_L1
error_L1:
	ret void
}
define dso_local void @ok() #0 {
ok_L0:
	call void @putch(i32 noundef 111)
	call void @putch(i32 noundef 107)
	call void @putch(i32 noundef 10)
	br label %ok_L1
ok_L1:
	ret void
}
define dso_local void @assert(i32 noundef %0) #0 {
assert_L0:
	%1 = alloca i32, align 4 ; cond
	store i32 %0, i32* %1, align 8
	%2 = load i32, i32* %1, align 4
	%3 = icmp ne i32 %2, 0
	br i1 %3, label %assert_L2, label %assert_L1
assert_L1:
	call void @error()
	br label %assert_L3
assert_L2:
	call void @ok()
	br label %assert_L3
assert_L3:
	br label %assert_L4
assert_L4:
	ret void
}
define dso_local void @assert_not(i32 noundef %0) #0 {
assert_not_L0:
	%1 = alloca i32, align 4 ; cond
	store i32 %0, i32* %1, align 8
	%2 = load i32, i32* %1, align 4
	%3 = icmp ne i32 %2, 0
	br i1 %3, label %assert_not_L1, label %assert_not_L2
assert_not_L1:
	call void @error()
	br label %assert_not_L3
assert_not_L2:
	call void @ok()
	br label %assert_not_L3
assert_not_L3:
	br label %assert_not_L4
assert_not_L4:
	ret void
}
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = load float, float* @HEX2, align 4
	%2 = load float, float* @FACT, align 4
	%3 = call i32 @float_eq(float noundef %1, float noundef %2)
	call void @assert_not(i32 noundef %3)
	%4 = load float, float* @EVAL1, align 4
	%5 = load float, float* @EVAL2, align 4
	%6 = call i32 @float_eq(float noundef %4, float noundef %5)
	call void @assert_not(i32 noundef %6)
	%7 = load float, float* @EVAL2, align 4
	%8 = load float, float* @EVAL3, align 4
	%9 = call i32 @float_eq(float noundef %7, float noundef %8)
	call void @assert(i32 noundef %9)
	%10 = load float, float* @RADIUS, align 4
	%11 = fptosi float %10 to i32
	%12 = call float @circle_area(i32 noundef %11)
	%13 = load i32, i32* @FIVE, align 4
	%14 = call float @circle_area(i32 noundef %13)
	%15 = call i32 @float_eq(float noundef %12, float noundef %14)
	call void @assert(i32 noundef %15)
	%16 = load float, float* @CONV1, align 4
	%17 = load float, float* @CONV2, align 4
	%18 = call i32 @float_eq(float noundef %16, float noundef %17)
	call void @assert_not(i32 noundef %18)
	%19 = fcmp one float 0x3ff8000000000000, 0x0000000000000000
	br i1 %19, label %main_L1, label %main_L2
main_L1:
	call void @ok()
	br label %main_L2
main_L2:
	%20 = fcmp one float 0x400a666660000000, 0x0000000000000000
	br i1 %20, label %main_L3, label %main_L4
main_L3:
	call void @ok()
	br label %main_L4
main_L4:
	%21 = fcmp one float 0x0000000000000000, 0x0000000000000000
	br i1 %21, label %main_L5, label %main_L7
main_L5:
	%22 = icmp ne i32 3, 0
	br i1 %22, label %main_L6, label %main_L7
main_L6:
	call void @error()
	br label %main_L7
main_L7:
	%23 = icmp ne i32 0, 0
	br i1 %23, label %main_L9, label %main_L8
main_L8:
	%24 = fcmp one float 0x3fd3333340000000, 0x0000000000000000
	br i1 %24, label %main_L9, label %main_L10
main_L9:
	call void @ok()
	br label %main_L10
main_L10:
	%25 = alloca i32, align 4 ; i
	store i32 1, i32* %25, align 4
	%26 = alloca i32, align 4 ; p
	store i32 0, i32* %26, align 4
	%27 = alloca [10 x float], align 16 ; arr
	%28 = bitcast [10 x float]* %27 to i8*
	%29 = bitcast [10 x float]* @__const.main.arr.0 to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %28, i8* align 16 %29, i64 40, i1 false)
	%30 = alloca i32, align 4 ; len
	%31 = getelementptr inbounds [10 x float], [10 x float]* %27, i64 0, i64 0
	%32 = call i32 @getfarray(float* noundef %31)
	store i32 %32, i32* %30, align 4
	br label %main_L11
main_L11:
	%33 = load i32, i32* %25, align 4
	%34 = load i32, i32* @MAX, align 4
	%35 = icmp slt i32 %33, %34
	br i1 %35, label %main_L12, label %main_L13
main_L12:
	%36 = alloca float, align 4 ; input
	%37 = call float @getfloat()
	store float %37, float* %36, align 4
	%38 = alloca float, align 4 ; area
	%39 = load float, float* @PI, align 4
	%40 = load float, float* %36, align 4
	%41 = fmul float %39, %40
	%42 = load float, float* %36, align 4
	%43 = fmul float %41, %42
	store float %43, float* %38, align 4
	%44 = alloca float, align 4 ; area_trunc
	%45 = load float, float* %36, align 4
	%46 = fptosi float %45 to i32
	%47 = call float @circle_area(i32 noundef %46)
	store float %47, float* %44, align 4
	%48 = load i32, i32* %26, align 4
	%49 = sext i32 %48 to i64
	%50 = getelementptr inbounds [10 x float], [10 x float]* %27, i64 0, i64 %49
	%51 = load float, float* %50, align 4
	%52 = load float, float* %36, align 4
	%53 = fadd float %51, %52
	%54 = load i32, i32* %26, align 4
	%55 = sext i32 %54 to i64
	%56 = getelementptr inbounds [10 x float], [10 x float]* %27, i64 0, i64 %55
	store float %53, float* %56, align 4
	%57 = load float, float* %38, align 4
	call void @putfloat(float noundef %57)
	call void @putch(i32 noundef 32)
	%58 = load float, float* %44, align 4
	%59 = fptosi float %58 to i32
	call void @putint(i32 noundef %59)
	call void @putch(i32 noundef 10)
	%60 = fsub float 0x0000000000000000, 0x4024000000000000
	%61 = fsub float 0x0000000000000000, %60
	%62 = load i32, i32* %25, align 4
	%63 = sitofp i32 %62 to float
	%64 = fmul float %63, %61
	%65 = fptosi float %64 to i32
	store i32 %65, i32* %25, align 4
	%66 = load i32, i32* %26, align 4
	%67 = add nsw i32 %66, 1
	store i32 %67, i32* %26, align 4
	br label %main_L11
main_L13:
	%68 = load i32, i32* %30, align 4
	%69 = getelementptr inbounds [10 x float], [10 x float]* %27, i64 0, i64 0
	call void @putfarray(i32 noundef %68, float* noundef %69)
	store i32 0, i32* %0, align 4
	br label %main_L14
main_L14:
	%70 = load i32, i32* %0, align 4
	ret i32 %70
}
