; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

@to = private unnamed_addr global [5005 x i32] zeroinitializer, align 4
@next = private unnamed_addr global [5005 x i32] zeroinitializer, align 4
@head = private unnamed_addr global [1005 x i32] zeroinitializer, align 4
@cnt = private unnamed_addr global i32 0, align 4
define dso_local void @add_edge(i32 noundef %0, i32 noundef %1) #0 {
add_edge_L0:
	%2 = alloca i32, align 4 ; from
	%3 = alloca i32, align 4 ; To
	store i32 %0, i32* %2, align 4
	store i32 %1, i32* %3, align 4
	%4 = load i32, i32* %3, align 4
	%5 = load i32, i32* @cnt, align 4
	%6 = sext i32 %5 to i64
	%7 = getelementptr inbounds [5005 x i32], [5005 x i32]* @to, i64 0, i64 %6
	store i32 %4, i32* %7, align 4
	%8 = load i32, i32* %2, align 4
	%9 = sext i32 %8 to i64
	%10 = getelementptr inbounds [1005 x i32], [1005 x i32]* @head, i64 0, i64 %9
	%11 = load i32, i32* %10, align 4
	%12 = load i32, i32* @cnt, align 4
	%13 = sext i32 %12 to i64
	%14 = getelementptr inbounds [5005 x i32], [5005 x i32]* @next, i64 0, i64 %13
	store i32 %11, i32* %14, align 4
	%15 = load i32, i32* @cnt, align 4
	%16 = load i32, i32* %2, align 4
	%17 = sext i32 %16 to i64
	%18 = getelementptr inbounds [1005 x i32], [1005 x i32]* @head, i64 0, i64 %17
	store i32 %15, i32* %18, align 4
	%19 = load i32, i32* @cnt, align 4
	%20 = add nsw i32 %19, 1
	store i32 %20, i32* @cnt, align 4
	%21 = load i32, i32* %2, align 4
	%22 = load i32, i32* @cnt, align 4
	%23 = sext i32 %22 to i64
	%24 = getelementptr inbounds [5005 x i32], [5005 x i32]* @to, i64 0, i64 %23
	store i32 %21, i32* %24, align 4
	%25 = load i32, i32* %3, align 4
	%26 = sext i32 %25 to i64
	%27 = getelementptr inbounds [1005 x i32], [1005 x i32]* @head, i64 0, i64 %26
	%28 = load i32, i32* %27, align 4
	%29 = load i32, i32* @cnt, align 4
	%30 = sext i32 %29 to i64
	%31 = getelementptr inbounds [5005 x i32], [5005 x i32]* @next, i64 0, i64 %30
	store i32 %28, i32* %31, align 4
	%32 = load i32, i32* @cnt, align 4
	%33 = load i32, i32* %3, align 4
	%34 = sext i32 %33 to i64
	%35 = getelementptr inbounds [1005 x i32], [1005 x i32]* @head, i64 0, i64 %34
	store i32 %32, i32* %35, align 4
	%36 = load i32, i32* @cnt, align 4
	%37 = add nsw i32 %36, 1
	store i32 %37, i32* @cnt, align 4
	br label %add_edge_L1
add_edge_L1:
	ret void
}
