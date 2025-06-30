.arch armv8-a
.text
.align 2
.cpu generic+fp+simd

.global main
.type main, %function
.align 2
main:
	sub sp,sp,#80
	stp x29,x30,[sp, #64]
	add x29,sp,#64
	str wzr,[sp,#8]
	add x0,sp,#16
	str wzr,[x0, #0]
	str wzr,[x0, #4]
	str wzr,[x0, #8]
	add x0,sp,#32
	str wzr,[x0, #0]
	str wzr,[x0, #4]
	str wzr,[x0, #8]
	str wzr,[x0, #12]
	str wzr,[x0, #16]
	str wzr,[x0, #20]
	add x2,sp,#56
	adrp x0,__const.main.complex.0
	add x0,x0,:lo12:__const.main.complex.0
	ldr w10,[x0, #0]
	str w10,[x2, #0]
	ldr w10,[x0, #4]
	str w10,[x2, #4]
	mov w0,#100
	add x10,sp,#16
	str w0,[x10]
	mov w0,#200
	add x10,sp,#32
	str w0,[x10]
	add x10,sp,#16
	ldr w1,[x10]
	mov w0,w1
	bl putint
	mov w0,#10
	bl putch
	add x10,sp,#32
	ldr w1,[x10]
	mov w0,w1
	bl putint
	mov w0,#10
	bl putch
	str wzr,[sp,#8]
	add x10,sp,#8
	ldr w0,[x10]
	ldr w0,[sp,#8]
	ldp x29,x30,[sp, #64]
	add sp,sp,#80
	ret

.type __const.main.complex.0, @object
.data
.global __const.main.complex.0
.align 16
__const.main.complex.0:
.word 1
.word 2
.size __const.main.complex.0, 8
