; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca i32, align 4 ; sum
	store i32 0, i32* %1, align 4
	%2 = alloca i32, align 4 ; i
	store i32 0, i32* %2, align 4
	br label %main_L1
main_L1:
	%3 = load i32, i32* %2, align 4
	%4 = icmp slt i32 %3, 20
	br i1 %4, label %main_L2, label %main_L30
main_L2:
	%5 = alloca i32, align 4 ; j
	store i32 0, i32* %5, align 4
	br label %main_L3
main_L3:
	%6 = load i32, i32* %5, align 4
	%7 = icmp slt i32 %6, 10
	br i1 %7, label %main_L4, label %main_L29
main_L4:
	%8 = alloca i32, align 4 ; k
	store i32 0, i32* %8, align 4
	br label %main_L5
main_L5:
	%9 = load i32, i32* %8, align 4
	%10 = icmp slt i32 %9, 5
	br i1 %10, label %main_L6, label %main_L28
main_L6:
	%11 = alloca i32, align 4 ; m
	store i32 0, i32* %11, align 4
	br label %main_L7
main_L7:
	%12 = load i32, i32* %11, align 4
	%13 = icmp slt i32 %12, 3
	br i1 %13, label %main_L8, label %main_L21
main_L8:
	%14 = load i32, i32* %11, align 4
	%15 = add nsw i32 %14, 1
	%16 = icmp sge i32 %15, 3
	br i1 %16, label %main_L9, label %main_L17
main_L9:
	%17 = load i32, i32* %11, align 4
	%18 = icmp ne i32 %17, 0
	br i1 %18, label %main_L10, label %main_L16
main_L10:
	%19 = load i32, i32* %11, align 4
	%20 = icmp ne i32 %19, 0
	br i1 %20, label %main_L12, label %main_L11
main_L11:
	%21 = load i32, i32* %11, align 4
	%22 = icmp ne i32 %21, 0
	br i1 %22, label %main_L15, label %main_L12
main_L12:
	%23 = sub nsw i32 0, 1
	%24 = load i32, i32* %11, align 4
	%25 = sub nsw i32 %24, %23
	%26 = icmp sge i32 %25, 3
	br i1 %26, label %main_L13, label %main_L14
main_L13:
	br label %main_L21
main_L14:
	br label %main_L15
main_L15:
	br label %main_L16
main_L16:
	br label %main_L17
main_L17:
	%27 = alloca i32, align 4 ; n
	store i32 0, i32* %27, align 4
	br label %main_L18
main_L18:
	%28 = load i32, i32* %27, align 4
	%29 = icmp slt i32 %28, 2
	br i1 %29, label %main_L19, label %main_L20
main_L19:
	%30 = load i32, i32* %27, align 4
	%31 = add nsw i32 %30, 1
	store i32 %31, i32* %27, align 4
	br label %main_L18
main_L20:
	%32 = load i32, i32* %11, align 4
	%33 = add nsw i32 %32, 1
	store i32 %33, i32* %11, align 4
	%34 = load i32, i32* %1, align 4
	%35 = add nsw i32 %34, 1
	store i32 %35, i32* %1, align 4
	br label %main_L7
main_L21:
	br label %main_L22
main_L22:
	%36 = icmp ne i32 1, 0
	br i1 %36, label %main_L23, label %main_L27
main_L23:
	br label %main_L24
main_L24:
	%37 = icmp ne i32 1, 0
	br i1 %37, label %main_L25, label %main_L26
main_L25:
	br label %main_L26
main_L26:
	br label %main_L27
main_L27:
	%38 = load i32, i32* %8, align 4
	%39 = add nsw i32 %38, 1
	store i32 %39, i32* %8, align 4
	br label %main_L5
main_L28:
	%40 = load i32, i32* %5, align 4
	%41 = add nsw i32 %40, 1
	store i32 %41, i32* %5, align 4
	br label %main_L3
main_L29:
	%42 = load i32, i32* %2, align 4
	%43 = add nsw i32 %42, 1
	store i32 %43, i32* %2, align 4
	br label %main_L1
main_L30:
	%44 = load i32, i32* %1, align 4
	store i32 %44, i32* %0, align 4
	br label %main_L31
main_L31:
	%45 = load i32, i32* %0, align 4
	ret i32 %45
}
