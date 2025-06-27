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
	mov x0,x1
	mov w0,#42
	add x10,sp,#68
	str w0,[x10]
	add x10,sp,#68
	ldr w0,[x10]
	add x10,sp,#76
	str w0,[x10]
	add x10,sp,#76
	ldr w0,[x10]
	ldr w0,[sp,#76]
	add sp,sp,#80
	ret

