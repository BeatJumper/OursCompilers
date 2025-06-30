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

@__nested_array_0 = dso_local constant [2 x i32] [i32 1, i32 2], align 16
@__nested_array_1 = dso_local constant [2 x i32] [i32 3, i32 4], align 16
@__const.main.complex2d.0 = dso_local constant [2 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4]], align 16
define dso_local i32 @process2D([3 x i32]* noundef %0, i32 noundef %1) #0 {
process2D_L0:
	%2 = alloca [3 x i32]*, align 8 ; arr
	%3 = alloca i32, align 4 ; rows
	store [3 x i32]* %0, [3 x i32]** %2, align 8
	store i32 %1, i32* %3, align 8
	%4 = alloca i32, align 4 ; __ret
	%5 = alloca i32, align 4 ; sum
	store i32 0, i32* %5, align 4
	%6 = alloca i32, align 4 ; i
	store i32 0, i32* %6, align 4
	br label %process2D_L1
process2D_L1:
	%7 = load i32, i32* %6, align 4
	%8 = load i32, i32* %3, align 4
	%9 = icmp slt i32 %7, %8
	br i1 %9, label %process2D_L2, label %process2D_L6
process2D_L2:
	%10 = alloca i32, align 4 ; j
	store i32 0, i32* %10, align 4
	br label %process2D_L3
process2D_L3:
	%11 = load i32, i32* %10, align 4
	%12 = icmp slt i32 %11, 3
	br i1 %12, label %process2D_L4, label %process2D_L5
process2D_L4:
	%13 = load [3 x i32]*, [3 x i32]** %2, align 8
	%14 = load i32, i32* %6, align 4
	%15 = sext i32 %14 to i64
	%16 = getelementptr inbounds [3 x i32], [3 x i32]* %13, i64 %15
	%17 = load i32, i32* %10, align 4
	%18 = sext i32 %17 to i64
	%19 = getelementptr inbounds [3 x i32], [3 x i32]* %16, i64 0, i64 %18
	%20 = load i32, i32* %5, align 4
	%21 = load i32, i32* %19, align 4
	%22 = add nsw i32 %20, %21
	store i32 %22, i32* %5, align 4
	%23 = load i32, i32* %10, align 4
	%24 = add nsw i32 %23, 1
	store i32 %24, i32* %10, align 4
	br label %process2D_L3
process2D_L5:
	%25 = load i32, i32* %6, align 4
	%26 = add nsw i32 %25, 1
	store i32 %26, i32* %6, align 4
	br label %process2D_L1
process2D_L6:
	%27 = load i32, i32* %5, align 8
	store i32 %27, i32* %4, align 4
	br label %process2D_L7
process2D_L7:
	%28 = load i32, i32* %4, align 4
	ret i32 %28
}
define dso_local i32 @process3D([2 x [2 x i32]]* noundef %0, i32 noundef %1) #0 {
process3D_L0:
	%2 = alloca [2 x [2 x i32]]*, align 8 ; arr
	%3 = alloca i32, align 4 ; depth
	store [2 x [2 x i32]]* %0, [2 x [2 x i32]]** %2, align 8
	store i32 %1, i32* %3, align 8
	%4 = alloca i32, align 4 ; __ret
	%5 = alloca i32, align 4 ; sum
	store i32 0, i32* %5, align 4
	%6 = alloca i32, align 4 ; i
	store i32 0, i32* %6, align 4
	br label %process3D_L1
process3D_L1:
	%7 = load i32, i32* %6, align 4
	%8 = load i32, i32* %3, align 4
	%9 = icmp slt i32 %7, %8
	br i1 %9, label %process3D_L2, label %process3D_L9
process3D_L2:
	%10 = alloca i32, align 4 ; j
	store i32 0, i32* %10, align 4
	br label %process3D_L3
process3D_L3:
	%11 = load i32, i32* %10, align 4
	%12 = icmp slt i32 %11, 2
	br i1 %12, label %process3D_L4, label %process3D_L8
process3D_L4:
	%13 = alloca i32, align 4 ; k
	store i32 0, i32* %13, align 4
	br label %process3D_L5
process3D_L5:
	%14 = load i32, i32* %13, align 4
	%15 = icmp slt i32 %14, 2
	br i1 %15, label %process3D_L6, label %process3D_L7
process3D_L6:
	%16 = load [2 x [2 x i32]]*, [2 x [2 x i32]]** %2, align 8
	%17 = load i32, i32* %6, align 4
	%18 = sext i32 %17 to i64
	%19 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %16, i64 %18
	%20 = load i32, i32* %10, align 4
	%21 = sext i32 %20 to i64
	%22 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %19, i64 0, i64 %21
	%23 = load i32, i32* %13, align 4
	%24 = sext i32 %23 to i64
	%25 = getelementptr inbounds [2 x i32], [2 x i32]* %22, i64 0, i64 %24
	%26 = load i32, i32* %5, align 4
	%27 = load i32, i32* %25, align 4
	%28 = add nsw i32 %26, %27
	store i32 %28, i32* %5, align 4
	%29 = load i32, i32* %13, align 4
	%30 = add nsw i32 %29, 1
	store i32 %30, i32* %13, align 4
	br label %process3D_L5
process3D_L7:
	%31 = load i32, i32* %10, align 4
	%32 = add nsw i32 %31, 1
	store i32 %32, i32* %10, align 4
	br label %process3D_L3
process3D_L8:
	%33 = load i32, i32* %6, align 4
	%34 = add nsw i32 %33, 1
	store i32 %34, i32* %6, align 4
	br label %process3D_L1
process3D_L9:
	%35 = load i32, i32* %5, align 8
	store i32 %35, i32* %4, align 4
	br label %process3D_L10
process3D_L10:
	%36 = load i32, i32* %4, align 4
	ret i32 %36
}
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca [2 x [3 x i32]], align 16 ; array2d
	%2 = bitcast [2 x [3 x i32]]* %1 to i8*
	call void @llvm.memset.p0i8.i64(i8* align 16 %2, i8 0, i64 24, i1 false)
	%3 = alloca [3 x [2 x [2 x i32]]], align 16 ; array3d
	%4 = bitcast [3 x [2 x [2 x i32]]]* %3 to i8*
	call void @llvm.memset.p0i8.i64(i8* align 16 %4, i8 0, i64 48, i1 false)
	%5 = alloca [2 x [2 x i32]], align 16 ; complex2d
	%6 = bitcast [2 x [2 x i32]]* %5 to i8*
	%7 = bitcast [2 x [2 x i32]]* @__const.main.complex2d.0 to i8*
	call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %6, i8* align 16 %7, i64 16, i1 false)
	%8 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 0
	%9 = getelementptr inbounds [3 x i32], [3 x i32]* %8, i64 0, i64 0
	store i32 1, i32* %9, align 4
	%10 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 0
	%11 = getelementptr inbounds [3 x i32], [3 x i32]* %10, i64 0, i64 1
	store i32 2, i32* %11, align 4
	%12 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 0
	%13 = getelementptr inbounds [3 x i32], [3 x i32]* %12, i64 0, i64 2
	store i32 3, i32* %13, align 4
	%14 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 1
	%15 = getelementptr inbounds [3 x i32], [3 x i32]* %14, i64 0, i64 0
	store i32 4, i32* %15, align 4
	%16 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 1
	%17 = getelementptr inbounds [3 x i32], [3 x i32]* %16, i64 0, i64 1
	store i32 5, i32* %17, align 4
	%18 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 1
	%19 = getelementptr inbounds [3 x i32], [3 x i32]* %18, i64 0, i64 2
	store i32 6, i32* %19, align 4
	%20 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
	%21 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %20, i64 0, i64 0
	%22 = getelementptr inbounds [2 x i32], [2 x i32]* %21, i64 0, i64 0
	store i32 10, i32* %22, align 4
	%23 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
	%24 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %23, i64 0, i64 0
	%25 = getelementptr inbounds [2 x i32], [2 x i32]* %24, i64 0, i64 1
	store i32 20, i32* %25, align 4
	%26 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
	%27 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %26, i64 0, i64 1
	%28 = getelementptr inbounds [2 x i32], [2 x i32]* %27, i64 0, i64 0
	store i32 30, i32* %28, align 4
	%29 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
	%30 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %29, i64 0, i64 1
	%31 = getelementptr inbounds [2 x i32], [2 x i32]* %30, i64 0, i64 1
	store i32 40, i32* %31, align 4
	%32 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 1
	%33 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %32, i64 0, i64 0
	%34 = getelementptr inbounds [2 x i32], [2 x i32]* %33, i64 0, i64 0
	store i32 50, i32* %34, align 4
	%35 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 1
	%36 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %35, i64 0, i64 0
	%37 = getelementptr inbounds [2 x i32], [2 x i32]* %36, i64 0, i64 1
	store i32 60, i32* %37, align 4
	%38 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 1
	%39 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %38, i64 0, i64 1
	%40 = getelementptr inbounds [2 x i32], [2 x i32]* %39, i64 0, i64 0
	store i32 70, i32* %40, align 4
	%41 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 1
	%42 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %41, i64 0, i64 1
	%43 = getelementptr inbounds [2 x i32], [2 x i32]* %42, i64 0, i64 1
	store i32 80, i32* %43, align 4
	%44 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 2
	%45 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %44, i64 0, i64 0
	%46 = getelementptr inbounds [2 x i32], [2 x i32]* %45, i64 0, i64 0
	store i32 90, i32* %46, align 4
	%47 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 2
	%48 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %47, i64 0, i64 0
	%49 = getelementptr inbounds [2 x i32], [2 x i32]* %48, i64 0, i64 1
	store i32 100, i32* %49, align 4
	%50 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 2
	%51 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %50, i64 0, i64 1
	%52 = getelementptr inbounds [2 x i32], [2 x i32]* %51, i64 0, i64 0
	store i32 110, i32* %52, align 4
	%53 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 2
	%54 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %53, i64 0, i64 1
	%55 = getelementptr inbounds [2 x i32], [2 x i32]* %54, i64 0, i64 1
	store i32 120, i32* %55, align 4
	%56 = alloca i32, align 4 ; sum2d
	%57 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %1, i64 0, i64 0
	%58 = call i32 @process2D([3 x i32]* noundef %57, i32 noundef 2)
	store i32 %58, i32* %56, align 4
	%59 = load i32, i32* %56, align 4
	call void @putint(i32 noundef %59)
	call void @putch(i32 noundef 10)
	%60 = alloca i32, align 4 ; sum3d
	%61 = getelementptr inbounds [3 x [2 x [2 x i32]]], [3 x [2 x [2 x i32]]]* %3, i64 0, i64 0
	%62 = call i32 @process3D([2 x [2 x i32]]* noundef %61, i32 noundef 3)
	store i32 %62, i32* %60, align 4
	%63 = load i32, i32* %60, align 4
	call void @putint(i32 noundef %63)
	call void @putch(i32 noundef 10)
	%64 = alloca i32, align 4 ; complex_sum
	%65 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %5, i64 0, i64 0
	%66 = getelementptr inbounds [2 x i32], [2 x i32]* %65, i64 0, i64 0
	%67 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %5, i64 0, i64 0
	%68 = getelementptr inbounds [2 x i32], [2 x i32]* %67, i64 0, i64 1
	%69 = load i32, i32* %66, align 4
	%70 = load i32, i32* %68, align 4
	%71 = add nsw i32 %69, %70
	%72 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %5, i64 0, i64 1
	%73 = getelementptr inbounds [2 x i32], [2 x i32]* %72, i64 0, i64 0
	%74 = load i32, i32* %73, align 4
	%75 = add nsw i32 %71, %74
	%76 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %5, i64 0, i64 1
	%77 = getelementptr inbounds [2 x i32], [2 x i32]* %76, i64 0, i64 1
	%78 = load i32, i32* %77, align 4
	%79 = add nsw i32 %75, %78
	store i32 %79, i32* %64, align 4
	%80 = load i32, i32* %64, align 4
	call void @putint(i32 noundef %80)
	call void @putch(i32 noundef 10)
	store i32 0, i32* %0, align 4
	br label %main_L1
main_L1:
	%81 = load i32, i32* %0, align 4
	ret i32 %81
}
