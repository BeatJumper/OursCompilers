.arch armv8-a
.text
.align 2
.cpu generic+fp+simd

.global main
.type main, %function
.align 2
main:
	sub sp,sp,#128
	str wzr,[sp,#124]
	mov x1,x2
	adrp x0,__const.main.d
	add x0,x0,:lo12:__const.main.d
	ldr w2,[x0, #0]
	str w2,[x1, #0]
	ldr w2,[x0, #4]
	str w2,[x1, #4]
	ldr w2,[x0, #8]
	str w2,[x1, #8]
	ldr w2,[x0, #12]
	str w2,[x1, #12]
	ldr w2,[x0, #16]
	str w2,[x1, #16]
	ldr w2,[x0, #20]
	str w2,[x1, #20]
	ldr w2,[x0, #24]
	str w2,[x1, #24]
	ldr w2,[x0, #28]
	str w2,[x1, #28]
	add x10,sp,#92
	ldr w0,[x10]
	add x10,sp,#124
	str w0,[x10]
	add x10,sp,#124
	ldr w0,[x10]
	ldr w0,[sp,#124]
	add sp,sp,#128
	ret

.type __const.main.d, @object
.data
.global __const.main.d
.align 16
__const.main.d:
.word 1
.word 2
.word 3
.word 4
.word 5
.word 6
.word 7
.word 8
.size __const.main.d, 32
