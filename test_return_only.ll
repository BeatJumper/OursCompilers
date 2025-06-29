; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

define dso_local i32 @test() #0 {
test_L0:
	%0 = alloca i32, align 4 ; __ret
	store i32 42, i32* %0, align 4
	br label %test_L1
test_L1:
	%1 = load i32, i32* %0, align 4
	ret i32 %1
}
