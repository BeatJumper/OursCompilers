; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

define dso_local void @test1(i32* noundef %0) #0 {
test1_L0:
	%1 = alloca i32*, align 8 ; arr
	store i32* %0, i32* %1, align 8
	%2 = getelementptr inbounds i32, i32* %1, i64 0
	store i32 1, i32* %2, align 4
test1_L1:
	ret void
}
define dso_local void @test2([4 x i32]* noundef %0) #0 {
test2_L0:
	%1 = alloca [4 x i32]*, align 8 ; matrix
	store [4 x i32]* %0, [4 x i32]* %1, align 8
	%2 = getelementptr inbounds [4 x i32], [4 x i32]* %1, i64 0, i64 0
	%3 = getelementptr inbounds i32, i32* %2, i64 0
	store i32 2, i32* %3, align 4
test2_L1:
	ret void
}
define dso_local void @test3([3 x [4 x i32]]* noundef %0) #0 {
test3_L0:
	%1 = alloca [3 x [4 x i32]]*, align 8 ; cube
	store [3 x [4 x i32]]* %0, [3 x [4 x i32]]* %1, align 8
	%2 = getelementptr inbounds [3 x [4 x i32]], [3 x [4 x i32]]* %1, i64 0, i64 0
	%3 = getelementptr inbounds [4 x i32], [4 x i32]* %2, i64 0, i64 1
	%4 = getelementptr inbounds i32, i32* %3, i64 2
	store i32 3, i32* %4, align 4
test3_L1:
	ret void
}
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca [5 x i32], align 16 ; arr
	%2 = bitcast [5 x i32]* %1 to i8*
	call void @llvm.memset.p0i8.i64(i8* align 16 %2, i8 0, i64 20, i1 false)
	%3 = alloca [2 x [4 x i32]], align 16 ; matrix
	%4 = bitcast [2 x [4 x i32]]* %3 to i8*
	call void @llvm.memset.p0i8.i64(i8* align 16 %4, i8 0, i64 32, i1 false)
	%5 = alloca [2 x [3 x [4 x i32]]], align 16 ; cube
	%6 = bitcast [2 x [3 x [4 x i32]]]* %5 to i8*
	call void @llvm.memset.p0i8.i64(i8* align 16 %6, i8 0, i64 96, i1 false)
	%7 = getelementptr inbounds [5 x i32], [5 x i32]* %1, i64 0, i64 0
	call void @test1(i32* noundef %7)
	%8 = getelementptr inbounds [2 x [4 x i32]], [2 x [4 x i32]]* %3, i64 0, i64 0
	call void @test2([4 x i32]* noundef %8)
	%9 = getelementptr inbounds [2 x [3 x [4 x i32]]], [2 x [3 x [4 x i32]]]* %5, i64 0, i64 0
	call void @test3([3 x [4 x i32]]* noundef %9)
	store i32 0, i32* %0, align 4
	br label %main_L1
main_L1:
	%10 = load i32, i32* %0, align 4
	ret i32 %10
}
