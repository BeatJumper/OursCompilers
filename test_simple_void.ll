; LLVM intrinsic function declarations
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

define dso_local void @simple() #0 {
simple_L0:
	%0 = alloca i32, align 4 ; x
	store i32 1, i32* %0, align 4
simple_L1:
	ret void
}
