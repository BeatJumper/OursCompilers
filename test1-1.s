.arch armv8-a
.text
.align 2
.cpu generic+fp+simd

.global main
.type main, %function
.align 2
main:
	sub sp,sp,#672
	str wzr,[sp,#668]
	add x0,sp,#4
	add x0,sp,#636
	adrp x1,__const.main.b
	add x1,x1,:lo12:__const.main.b
	ldr w2,[x1, #0]
	str w2,[x0, #0]
	ldr w2,[x1, #4]
	str w2,[x0, #4]
	ldr w2,[x1, #8]
	str w2,[x0, #8]
	ldr w2,[x1, #12]
	str w2,[x0, #12]
	ldr w2,[x1, #16]
	str w2,[x0, #16]
	ldr w2,[x1, #20]
	str w2,[x0, #20]
	ldr w2,[x1, #24]
	str w2,[x0, #24]
	ldr w2,[x1, #28]
	str w2,[x0, #28]
	add x1,sp,#604
	adrp x0,__const.main.c
	add x0,x0,:lo12:__const.main.c
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
	add x1,sp,#572
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
	add x10,sp,#592
	ldr w0,[x10]
	add x10,sp,#540
	str w0,[x10]
	add x10,sp,#624
	ldr w0,[x10]
	add x10,sp,#544
	str w0,[x10]
	mov w0,#3
	add x10,sp,#548
	str w0,[x10]
	mov w0,#4
	add x10,sp,#552
	str w0,[x10]
	mov w0,#5
	add x10,sp,#556
	str w0,[x10]
	mov w0,#6
	add x10,sp,#560
	str w0,[x10]
	mov w0,#7
	add x10,sp,#564
	str w0,[x10]
	mov w0,#8
	add x10,sp,#568
	str w0,[x10]
	add x10,sp,#568
	ldr w0,[x10]
	add x10,sp,#540
	ldr w2,[x10]
	add w1,w0,w2
	add x10,sp,#544
	ldr w2,[x10]
	add w0,w1,w2
	add x10,sp,#20
	ldr w1,[x10]
	add w0,w0,w1
	add x10,sp,#668
	str w0,[x10]
	add x10,sp,#668
	ldr w0,[x10]
	ldr w0,[sp,#668]
	add sp,sp,#672
	ret

.type __const.main.b, @object
.data
.global __const.main.b
.align 16
__const.main.b:
.word 1
.word 2
.word 3
.word 4
.word 5
.word 6
.word 7
.word 8
.size __const.main.b, 32
.type __const.main.c, @object
.global __const.main.c
.align 16
__const.main.c:
.word 1
.word 2
.word 3
.word 4
.word 5
.word 6
.word 7
.word 8
.size __const.main.c, 32
.type __const.main.d, @object
.global __const.main.d
.align 16
__const.main.d:
.word 1
.word 2
.word 3
.word 0
.word 5
.word 0
.word 7
.word 8
.size __const.main.d, 32
