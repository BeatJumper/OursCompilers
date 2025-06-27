.arch armv8-a
.text
.align 2
.cpu generic+fp+simd

.global main
.type main, %function
.align 2
main:
	sub sp,sp,#144
	str wzr,[sp,#140]
	add x0,sp,#4
	adrp x1,__const.main.a
	add x1,x1,:lo12:__const.main.a
	ldr w2,[x1, #0]
	str w2,[x0, #0]
	ldr w2,[x1, #4]
	str w2,[x0, #4]
	add x1,sp,#132
	adrp x2,__const.main.b
	add x2,x2,:lo12:__const.main.b
	ldr w2,[x2, #0]
	str w2,[x1, #0]
	ldr w2,[x2, #4]
	str w2,[x1, #4]
	add x10,sp,#4
	ldr w1,[x10]
	add x10,sp,#132
	ldr w0,[x10]
	add w0,w1,w0
	add x10,sp,#140
	str w0,[x10]
	add x10,sp,#140
	ldr w0,[x10]
	ldr w0,[sp,#140]
	add sp,sp,#144
	ret

.type __const.main.a, @object
.data
.global __const.main.a
.align 16
__const.main.a:
.word 1
.word 2
.size __const.main.a, 8
.type __const.main.b, @object
.global __const.main.b
.align 16
__const.main.b:
.word 3
.word 4
.size __const.main.b, 8
