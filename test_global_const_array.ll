; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

@__const.main.a = dso_local constant [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], align 16
@a = dso_local constant [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], align 16
define dso_local i32 @main() #0 {
main_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 0, i32* %0, align 4
	%1 = getelementptr inbounds [5 x i32], [5 x i32]* @a, i64 0, i64 4
	%2 = load i32, i32* %1, align 8
	store i32 %2, i32* %0, align 4
	br label %main_L1
main_L1:
	%3 = load i32, i32* %0, align 4
	ret i32 %3
}
