; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

define dso_local i32 @test() #0 {
test_L0:
	%0 = alloca i32, align 4 ; __ret
	%1 = alloca i32, align 4 ; f
	store i32 1, i32* %1, align 4
	%2 = load i32, i32* %1, align 4
	%3 = icmp ne i32 %2, 0
	br i1 %3, label %test_L1, label %test_L2
test_L1:
	%4 = sub nsw i32 0, 1
	store i32 %4, i32* %0, align 4
	br label %test_L3
test_L2:
	store i32 1, i32* %0, align 4
	br label %test_L3
test_L3:
	%5 = load i32, i32* %0, align 4
	ret i32 %5
}
