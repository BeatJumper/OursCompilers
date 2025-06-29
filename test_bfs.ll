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

@n = private unnamed_addr global i32 0, align 4
@m = private unnamed_addr global i32 0, align 4
@to = private unnamed_addr global [5005 x i32] zeroinitializer, align 4
@next = private unnamed_addr global [5005 x i32] zeroinitializer, align 4
@head = private unnamed_addr global [1005 x i32] zeroinitializer, align 4
@cnt = private unnamed_addr global i32 0, align 4
@que = private unnamed_addr global [1005 x i32] zeroinitializer, align 4
@h = private unnamed_addr global i32 0, align 4
@tail = private unnamed_addr global i32 0, align 4
@inq = private unnamed_addr global [1005 x i32] zeroinitializer, align 4
define dso_local i32 @quick_read() #0 {
quick_read_L0:
	%0 = alloca i32, align 4 ; __ret
	%1 = alloca i32, align 4 ; ch
	%2 = call i32 @getch()
	store i32 %2, i32* %1, align 4
	%3 = alloca i32, align 4 ; x
	store i32 0, i32* %3, align 4
	%4 = alloca i32, align 4 ; f
	store i32 0, i32* %4, align 4
	br label %quick_read_L1
quick_read_L1:
	%5 = load i32, i32* %1, align 4
	%6 = icmp slt i32 %5, 48
	br i1 %6, label %quick_read_L3, label %quick_read_L2
quick_read_L2:
	%7 = load i32, i32* %1, align 4
	%8 = icmp sgt i32 %7, 57
	br i1 %8, label %quick_read_L3, label %quick_read_L6
quick_read_L3:
	%9 = load i32, i32* %1, align 4
	%10 = icmp eq i32 %9, 45
	br i1 %10, label %quick_read_L4, label %quick_read_L5
quick_read_L4:
	store i32 1, i32* %4, align 4
	br label %quick_read_L5
quick_read_L5:
	%11 = call i32 @getch()
	store i32 %11, i32* %1, align 4
	br label %quick_read_L1
quick_read_L6:
	br label %quick_read_L7
quick_read_L7:
	%12 = load i32, i32* %1, align 4
	%13 = icmp sge i32 %12, 48
	br i1 %13, label %quick_read_L8, label %quick_read_L10
quick_read_L8:
	%14 = load i32, i32* %1, align 4
	%15 = icmp sle i32 %14, 57
	br i1 %15, label %quick_read_L9, label %quick_read_L10
quick_read_L9:
	%16 = load i32, i32* %3, align 4
	%17 = mul nsw i32 %16, 10
	%18 = load i32, i32* %1, align 4
	%19 = add nsw i32 %17, %18
	%20 = sub nsw i32 %19, 48
	store i32 %20, i32* %3, align 4
	%21 = call i32 @getch()
	store i32 %21, i32* %1, align 4
	br label %quick_read_L7
quick_read_L10:
	%22 = load i32, i32* %4, align 4
	%23 = icmp ne i32 %22, 0
	br i1 %23, label %quick_read_L11, label %quick_read_L12
quick_read_L11:
	%24 = load i32, i32* %3, align 4
	%25 = sub nsw i32 0, %24
	store i32 %25, i32* %0, align 4
	br label %quick_read_L13
quick_read_L12:
	%26 = load i32, i32* %3, align 4
	store i32 %26, i32* %0, align 4
	br label %quick_read_L13
quick_read_L13:
	%27 = load i32, i32* %0, align 4
	ret i32 %27
}
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
define dso_local void @init() #0 {
init_L0:
	%0 = alloca i32, align 4 ; i
	store i32 0, i32* %0, align 4
	br label %init_L1
init_L1:
	%1 = load i32, i32* %0, align 4
	%2 = icmp slt i32 %1, 1005
	br i1 %2, label %init_L2, label %init_L3
init_L2:
	%3 = sub nsw i32 0, 1
	%4 = load i32, i32* %0, align 4
	%5 = sext i32 %4 to i64
	%6 = getelementptr inbounds [1005 x i32], [1005 x i32]* @head, i64 0, i64 %5
	store i32 %3, i32* %6, align 4
	%7 = load i32, i32* %0, align 4
	%8 = add nsw i32 %7, 1
	store i32 %8, i32* %0, align 4
	br label %init_L1
init_L3:
	br label %init_L4
init_L4:
	ret void
}
define dso_local void @inqueue(i32 noundef %0) #0 {
inqueue_L0:
	%1 = alloca i32, align 4 ; x
	store i32 %0, i32* %1, align 4
	%2 = load i32, i32* %1, align 4
	%3 = sext i32 %2 to i64
	%4 = getelementptr inbounds [1005 x i32], [1005 x i32]* @inq, i64 0, i64 %3
	store i32 1, i32* %4, align 4
	%5 = load i32, i32* @tail, align 4
	%6 = add nsw i32 %5, 1
	store i32 %6, i32* @tail, align 4
	%7 = load i32, i32* %1, align 4
	%8 = load i32, i32* @tail, align 4
	%9 = sext i32 %8 to i64
	%10 = getelementptr inbounds [1005 x i32], [1005 x i32]* @que, i64 0, i64 %9
	store i32 %7, i32* %10, align 4
	br label %inqueue_L1
inqueue_L1:
	ret void
}
define dso_local i32 @pop_queue() #0 {
pop_queue_L0:
	%0 = alloca i32, align 4 ; __ret
	%1 = load i32, i32* @h, align 4
	%2 = add nsw i32 %1, 1
	store i32 %2, i32* @h, align 4
	%3 = alloca i32, align 4 ; res
	%4 = load i32, i32* @h, align 4
	%5 = sext i32 %4 to i64
	%6 = getelementptr inbounds [1005 x i32], [1005 x i32]* @que, i64 0, i64 %5
	%7 = load i32, i32* %6, align 4
	store i32 %7, i32* %3, align 4
	%8 = load i32, i32* @h, align 4
	%9 = sext i32 %8 to i64
	%10 = getelementptr inbounds [1005 x i32], [1005 x i32]* @que, i64 0, i64 %9
	%11 = load i32, i32* %10, align 8
	store i32 %11, i32* %0, align 4
	br label %pop_queue_L1
pop_queue_L1:
	%12 = load i32, i32* %0, align 4
	ret i32 %12
}
define dso_local i32 @same(i32 noundef %0, i32 noundef %1) #0 {
same_L0:
	%2 = alloca i32, align 4 ; s
	%3 = alloca i32, align 4 ; t
	store i32 %0, i32* %2, align 4
	store i32 %1, i32* %3, align 4
	%4 = alloca i32, align 4 ; __ret
	store i32 0, i32* @h, align 4
	store i32 0, i32* @tail, align 4
	%5 = load i32, i32* %2, align 4
	call void @inqueue(i32 noundef %5)
	%6 = alloca i32, align 4 ; res
	store i32 0, i32* %6, align 4
	br label %same_L1
same_L1:
	%7 = load i32, i32* @h, align 4
	%8 = load i32, i32* @tail, align 4
	%9 = icmp slt i32 %7, %8
	br i1 %9, label %same_L2, label %same_L10
same_L2:
	%10 = alloca i32, align 4 ; x
	%11 = call i32 @pop_queue()
	store i32 %11, i32* %10, align 4
	%12 = load i32, i32* %10, align 4
	%13 = load i32, i32* %3, align 4
	%14 = icmp eq i32 %12, %13
	br i1 %14, label %same_L3, label %same_L4
same_L3:
	store i32 1, i32* %6, align 4
	br label %same_L4
same_L4:
	%15 = alloca i32, align 4 ; i
	%16 = load i32, i32* %10, align 4
	%17 = sext i32 %16 to i64
	%18 = getelementptr inbounds [1005 x i32], [1005 x i32]* @head, i64 0, i64 %17
	%19 = load i32, i32* %18, align 4
	store i32 %19, i32* %15, align 4
	br label %same_L5
same_L5:
	%20 = load i32, i32* %15, align 4
	%21 = sub nsw i32 0, 1
	%22 = icmp ne i32 %20, %21
	br i1 %22, label %same_L6, label %same_L9
same_L6:
	%23 = load i32, i32* %15, align 4
	%24 = sext i32 %23 to i64
	%25 = getelementptr inbounds [5005 x i32], [5005 x i32]* @to, i64 0, i64 %24
	%26 = load i32, i32* %25, align 4
	%27 = sext i32 %26 to i64
	%28 = getelementptr inbounds [1005 x i32], [1005 x i32]* @inq, i64 0, i64 %27
	%29 = load i32, i32* %28, align 4
	%30 = icmp ne i32 %29, 0
	br i1 %30, label %same_L8, label %same_L7
same_L7:
	%31 = load i32, i32* %15, align 4
	%32 = sext i32 %31 to i64
	%33 = getelementptr inbounds [5005 x i32], [5005 x i32]* @to, i64 0, i64 %32
	%34 = load i32, i32* %33, align 4
	call void @inqueue(i32 noundef %34)
	br label %same_L8
same_L8:
	%35 = load i32, i32* %15, align 4
	%36 = sext i32 %35 to i64
	%37 = getelementptr inbounds [5005 x i32], [5005 x i32]* @next, i64 0, i64 %36
	%38 = load i32, i32* %37, align 4
	store i32 %38, i32* %15, align 4
	br label %same_L5
same_L9:
	br label %same_L1
same_L10:
	%39 = alloca i32, align 4 ; i
	store i32 0, i32* %39, align 4
	br label %same_L11
same_L11:
	%40 = load i32, i32* %39, align 4
	%41 = load i32, i32* @tail, align 4
	%42 = icmp sle i32 %40, %41
	br i1 %42, label %same_L12, label %same_L13
same_L12:
	%43 = load i32, i32* %39, align 4
	%44 = sext i32 %43 to i64
	%45 = getelementptr inbounds [1005 x i32], [1005 x i32]* @que, i64 0, i64 %44
	%46 = load i32, i32* %45, align 4
	%47 = sext i32 %46 to i64
	%48 = getelementptr inbounds [1005 x i32], [1005 x i32]* @inq, i64 0, i64 %47
	store i32 0, i32* %48, align 4
	%49 = load i32, i32* %39, align 4
	%50 = add nsw i32 %49, 1
	store i32 %50, i32* %39, align 4
	br label %same_L11
same_L13:
	%51 = load i32, i32* %6, align 4
	store i32 %51, i32* %4, align 4
	br label %same_L14
same_L14:
	%52 = load i32, i32* %4, align 4
	ret i32 %52
}
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = call i32 @quick_read()
	store i32 %1, i32* @n, align 4
	%2 = call i32 @quick_read()
	store i32 %2, i32* @m, align 4
	call void @init()
	br label %main_L1
main_L1:
	%3 = load i32, i32* @m, align 4
	%4 = icmp ne i32 %3, 0
	br i1 %4, label %main_L2, label %main_L10
main_L2:
	%5 = alloca i32, align 4 ; ch
	%6 = call i32 @getch()
	store i32 %6, i32* %5, align 4
	br label %main_L3
main_L3:
	%7 = load i32, i32* %5, align 4
	%8 = icmp ne i32 %7, 81
	br i1 %8, label %main_L4, label %main_L6
main_L4:
	%9 = load i32, i32* %5, align 4
	%10 = icmp ne i32 %9, 85
	br i1 %10, label %main_L5, label %main_L6
main_L5:
	%11 = call i32 @getch()
	store i32 %11, i32* %5, align 4
	br label %main_L3
main_L6:
	%12 = load i32, i32* %5, align 4
	%13 = icmp eq i32 %12, 81
	br i1 %13, label %main_L7, label %main_L8
main_L7:
	%14 = alloca i32, align 4 ; x
	%15 = call i32 @quick_read()
	store i32 %15, i32* %14, align 4
	%16 = alloca i32, align 4 ; y
	%17 = call i32 @quick_read()
	store i32 %17, i32* %16, align 4
	%18 = load i32, i32* %14, align 4
	%19 = load i32, i32* %16, align 4
	%20 = call i32 @same(i32 noundef %18, i32 noundef %19)
	call void @putint(i32 noundef %20)
	call void @putch(i32 noundef 10)
	br label %main_L9
main_L8:
	%21 = alloca i32, align 4 ; x
	%22 = call i32 @quick_read()
	store i32 %22, i32* %21, align 4
	%23 = alloca i32, align 4 ; y
	%24 = call i32 @quick_read()
	store i32 %24, i32* %23, align 4
	%25 = load i32, i32* %21, align 4
	%26 = load i32, i32* %23, align 4
	call void @add_edge(i32 noundef %25, i32 noundef %26)
	br label %main_L9
main_L9:
	%27 = load i32, i32* @m, align 4
	%28 = sub nsw i32 %27, 1
	store i32 %28, i32* @m, align 4
	br label %main_L1
main_L10:
	store i32 0, i32* %0, align 4
	br label %main_L11
main_L11:
	%29 = load i32, i32* %0, align 4
	ret i32 %29
}
