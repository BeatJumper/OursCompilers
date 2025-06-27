.arch armv8-a
.text
.align 2
.cpu generic+fp+simd

.global main
.type main, %function
.align 2
main:
	sub sp,sp,#80
	str wzr,[sp,#76]
	add x0,sp,#16
	adrp x1,__const.main.a
	add x1,x1,:lo12:__const.main.a
	ldr w2,[x1, #0]
	str w2,[x0, #0]
	ldr w2,[x1, #4]
	str w2,[x0, #4]
	add x10,sp,#16
	ldr w1,[x10]
	add x10,sp,#20
	ldr w0,[x10]
	add w0,w1,w0
	add x10,sp,#76
	str w0,[x10]
	add x10,sp,#76
	ldr w0,[x10]
	ldr w0,[sp,#76]
	add sp,sp,#80
	ret

.type __const.main.a, @object
.data
.global __const.main.a
.align 16
__const.main.a:
.word 1
.word 2
.size __const.main.a, 8
