.arch armv8-a
.text
.align 2
.cpu generic+fp+simd

.global process2D
.type process2D, %function
.align 2
process2D:
	sub sp,sp,#96
	add x10,sp,#48
	str w0,[x10]
	add x10,sp,#56
	str w1,[x10]
	str wzr,[sp,#72]
	str wzr,[sp,#80]
.Lprocess2D_L1:
	add x10,sp,#80
	ldr w0,[x10]
	add x10,sp,#56
	ldr w1,[x10]
	cmp w0,w1
	cset w0,lt
	cmp w0,#0
	b.ne .Lprocess2D_L2
	b .Lprocess2D_L6
.Lprocess2D_L2:
	str wzr,[sp,#88]
.Lprocess2D_L3:
	add x10,sp,#88
	ldr w0,[x10]
	cmp w0,#3
	cset w0,lt
	cmp w0,#0
	b.ne .Lprocess2D_L4
	b .Lprocess2D_L5
.Lprocess2D_L4:
	add x10,sp,#48
	ldr w1,[x10]
	add x10,sp,#80
	ldr w0,[x10]
	add x10,sp,#88
	ldr w0,[x10]
	add x10,sp,#72
	ldr w1,[x10]
	add x10,sp,#116
	ldr w0,[x10]
	add w0,w1,w0
	add x10,sp,#72
	str w0,[x10]
	add x10,sp,#88
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#88
	str w0,[x10]
	b .Lprocess2D_L3
.Lprocess2D_L5:
	add x10,sp,#80
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#80
	str w0,[x10]
	b .Lprocess2D_L1
.Lprocess2D_L6:
	add x10,sp,#72
	ldr w0,[x10]
	add x10,sp,#64
	str w0,[x10]
	add x10,sp,#64
	ldr w0,[x10]
	ldr w0,[sp,#64]
	add sp,sp,#96
	ret

.global process3D
.type process3D, %function
.align 2
process3D:
	sub sp,sp,#112
	add x10,sp,#56
	str w1,[x10]
	add x10,sp,#64
	str w0,[x10]
	str wzr,[sp,#80]
	str wzr,[sp,#88]
.Lprocess3D_L1:
	add x10,sp,#88
	ldr w1,[x10]
	add x10,sp,#64
	ldr w0,[x10]
	cmp w1,w0
	cset w0,lt
	cmp w0,#0
	b.ne .Lprocess3D_L2
	b .Lprocess3D_L9
.Lprocess3D_L2:
	str wzr,[sp,#96]
.Lprocess3D_L3:
	add x10,sp,#96
	ldr w0,[x10]
	cmp w0,#2
	cset w0,lt
	cmp w0,#0
	b.ne .Lprocess3D_L4
	b .Lprocess3D_L8
.Lprocess3D_L4:
	str wzr,[sp,#104]
.Lprocess3D_L5:
	add x10,sp,#104
	ldr w0,[x10]
	cmp w0,#2
	cset w0,lt
	cmp w0,#0
	b.ne .Lprocess3D_L6
	b .Lprocess3D_L7
.Lprocess3D_L6:
	add x10,sp,#56
	ldr w1,[x10]
	add x10,sp,#88
	ldr w0,[x10]
	add x10,sp,#96
	ldr w0,[x10]
	add x10,sp,#104
	ldr w1,[x10]
	add x10,sp,#80
	ldr w1,[x10]
	add x10,sp,#140
	ldr w0,[x10]
	add w0,w1,w0
	add x10,sp,#80
	str w0,[x10]
	add x10,sp,#104
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#104
	str w0,[x10]
	b .Lprocess3D_L5
.Lprocess3D_L7:
	add x10,sp,#96
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#96
	str w0,[x10]
	b .Lprocess3D_L3
.Lprocess3D_L8:
	add x10,sp,#88
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#88
	str w0,[x10]
	b .Lprocess3D_L1
.Lprocess3D_L9:
	add x10,sp,#80
	ldr w0,[x10]
	add x10,sp,#72
	str w0,[x10]
	add x10,sp,#72
	ldr w0,[x10]
	ldr w0,[sp,#72]
	add sp,sp,#112
	ret

.global main
.type main, %function
.align 2
main:
	sub sp,sp,#176
	stp x29,x30,[sp, #160]
	add x29,sp,#160
	str wzr,[sp,#32]
	add x0,sp,#40
	str wzr,[x0, #0]
	str wzr,[x0, #4]
	str wzr,[x0, #8]
	str wzr,[x0, #12]
	str wzr,[x0, #16]
	str wzr,[x0, #20]
	add x0,sp,#64
	str wzr,[x0, #0]
	str wzr,[x0, #4]
	str wzr,[x0, #8]
	str wzr,[x0, #12]
	str wzr,[x0, #16]
	str wzr,[x0, #20]
	str wzr,[x0, #24]
	str wzr,[x0, #28]
	str wzr,[x0, #32]
	str wzr,[x0, #36]
	str wzr,[x0, #40]
	str wzr,[x0, #44]
	add x2,sp,#112
	adrp x0,__const.main.complex2d.0
	add x0,x0,:lo12:__const.main.complex2d.0
	ldr w10,[x0, #0]
	str w10,[x2, #0]
	ldr w10,[x0, #4]
	str w10,[x2, #4]
	ldr w10,[x0, #8]
	str w10,[x2, #8]
	ldr w10,[x0, #12]
	str w10,[x2, #12]
	mov w2,#1
	add x10,sp,#40
	str w2,[x10]
	mov w0,#2
	add x10,sp,#44
	str w0,[x10]
	mov w0,#3
	add x10,sp,#48
	str w0,[x10]
	mov w2,#4
	add x10,sp,#52
	str w2,[x10]
	mov w2,#5
	add x10,sp,#56
	str w2,[x10]
	mov w1,#6
	add x10,sp,#60
	str w1,[x10]
	mov w1,#10
	add x10,sp,#64
	str w1,[x10]
	mov w1,#20
	add x10,sp,#68
	str w1,[x10]
	mov w0,#30
	add x10,sp,#72
	str w0,[x10]
	mov w0,#40
	add x10,sp,#76
	str w0,[x10]
	mov w0,#50
	add x10,sp,#80
	str w0,[x10]
	mov w1,#60
	add x10,sp,#84
	str w1,[x10]
	mov w0,#70
	add x10,sp,#88
	str w0,[x10]
	mov w0,#80
	add x10,sp,#92
	str w0,[x10]
	mov w0,#90
	add x10,sp,#96
	str w0,[x10]
	mov w0,#100
	add x10,sp,#100
	str w0,[x10]
	mov w0,#110
	add x10,sp,#104
	str w0,[x10]
	mov w0,#120
	add x10,sp,#108
	str w0,[x10]
	mov w0,w2
	mov w1,#2
	bl process2D
	mov w1,w0
	add x10,sp,#128
	str w1,[x10]
	add x10,sp,#128
	ldr w1,[x10]
	mov w0,w1
	bl putint
	mov w0,#10
	bl putch
	mov w0,w2
	mov w1,#3
	bl process3D
	mov w1,w0
	add x10,sp,#136
	str w1,[x10]
	add x10,sp,#136
	ldr w1,[x10]
	mov w0,w1
	bl putint
	mov w0,#10
	bl putch
	add x10,sp,#112
	ldr w2,[x10]
	add x10,sp,#116
	ldr w0,[x10]
	add w0,w2,w0
	add x10,sp,#120
	ldr w2,[x10]
	add w1,w0,w2
	add x10,sp,#124
	ldr w0,[x10]
	add w0,w1,w0
	add x10,sp,#144
	str w0,[x10]
	add x10,sp,#144
	ldr w1,[x10]
	mov w0,w1
	bl putint
	mov w0,#10
	bl putch
	str wzr,[sp,#32]
	add x10,sp,#32
	ldr w0,[x10]
	ldr w0,[sp,#32]
	ldp x29,x30,[sp, #160]
	add sp,sp,#176
	ret

.type __nested_array_0, @object
.data
.global __nested_array_0
.align 16
__nested_array_0:
.word 1
.word 2
.size __nested_array_0, 8
.type __nested_array_1, @object
.global __nested_array_1
.align 16
__nested_array_1:
.word 3
.word 4
.size __nested_array_1, 8
.type __const.main.complex2d.0, @object
.global __const.main.complex2d.0
.align 16
__const.main.complex2d.0:
.word 1
.word 2
.word 3
.word 4
.size __const.main.complex2d.0, 16
