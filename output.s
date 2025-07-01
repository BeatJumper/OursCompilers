.arch armv8-a
.text
.align 2
.cpu generic+fp+simd

.global test_zext
.type test_zext, %function
.align 2
test_zext:
	sub sp,sp,#96
	mov w0,#5
	str w0,[sp,#64]
	mov w0,#10
	str w0,[sp,#72]
	mov w0,#3
	str w0,[sp,#80]
	ldr w0,[sp,#64]
	str w0,[sp,#88]
	ldr w0,[sp,#88]
	str w0,[sp,#56]
	ldr w0,[sp,#56]
	ldr w0,[sp,#56]
	add sp,sp,#96
	ret

.global main
.type main, %function
.align 2
main:
	sub sp,sp,#48
	stp x29,x30,[sp, #32]
	add x29,sp,#32
	str wzr,[sp,#24]
	bl test_zext
	mov w1,w0
	str w1,[sp,#24]
	ldr w0,[sp,#24]
	ldr w0,[sp,#24]
	ldp x29,x30,[sp, #32]
	add sp,sp,#48
	ret

