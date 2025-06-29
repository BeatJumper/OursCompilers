; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = alloca i32, align 4 ; i
	store i32 0, i32* %1, align 4
	br label %main_L1
main_L1:
	%2 = load i32, i32* %1, align 4
	%3 = icmp slt i32 %2, 5
	br i1 %3, label %main_L2, label %main_L5
main_L2:
	%4 = load i32, i32* %1, align 4
	%5 = icmp eq i32 %4, 2
	br i1 %5, label %main_L3, label %main_L4
main_L3:
	br label %main_L5
main_L4:
	%6 = load i32, i32* %1, align 4
	%7 = add nsw i32 %6, 1
	store i32 %7, i32* %1, align 4
	br label %main_L1
main_L5:
	%8 = load i32, i32* %1, align 4
	store i32 %8, i32* %0, align 4
	br label %main_L6
main_L6:
	%9 = load i32, i32* %0, align 4
	ret i32 %9
}
