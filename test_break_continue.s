.arch armv8-a
.text
.align 2
.cpu generic+fp+simd

.global main
.type main, %function
.align 2
main:
	sub sp,sp,#96
	str wzr,[sp,#28]
	str wzr,[sp,#36]
	str wzr,[sp,#44]
.Lmain_L2:
	add x10,sp,#44
	ldr w0,[x10]
	cmp w0,#20
	cset w0,lt
	cmp w0,#0
	b.ne .Lmain_L3
	b .Lmain_L37
.Lmain_L3:
	str wzr,[sp,#52]
.Lmain_L5:
	add x10,sp,#52
	ldr w0,[x10]
	cmp w0,#10
	cset w0,lt
	cmp w0,#0
	b.ne .Lmain_L6
	b .Lmain_L36
.Lmain_L6:
	str wzr,[sp,#60]
.Lmain_L8:
	add x10,sp,#60
	ldr w0,[x10]
	cmp w0,#5
	cset w0,lt
	cmp w0,#0
	b.ne .Lmain_L9
	b .Lmain_L35
.Lmain_L9:
	str wzr,[sp,#68]
.Lmain_L11:
	add x10,sp,#68
	ldr w0,[x10]
	cmp w0,#3
	cset w0,lt
	cmp w0,#0
	b.ne .Lmain_L12
	b .Lmain_L26
.Lmain_L12:
	add x10,sp,#68
	ldr w0,[x10]
	add w0,w0,#1
	cmp w0,#3
	cset w0,ge
	cmp w0,#0
	b.ne .Lmain_L13
	b .Lmain_L21
.Lmain_L13:
	add x10,sp,#68
	ldr w0,[x10]
	cmp w0,#0
	cset w0,ne
	cmp w0,#0
	b.ne .Lmain_L14
	b .Lmain_L20
.Lmain_L14:
	add x10,sp,#68
	ldr w0,[x10]
	cmp w0,#0
	cset w0,ne
	cmp w0,#0
	b.ne .Lmain_L16
	b .Lmain_L15
.Lmain_L15:
	add x10,sp,#68
	ldr w0,[x10]
	cmp w0,#0
	cset w0,ne
	cmp w0,#0
	b.ne .Lmain_L19
	b .Lmain_L16
.Lmain_L16:
	mov w0,#0
	subs w1,w0,#1
	add x10,sp,#68
	ldr w0,[x10]
	subs w0,w0,w1
	cmp w0,#3
	cset w0,ge
	cmp w0,#0
	b.ne .Lmain_L17
	b .Lmain_L18
.Lmain_L17:
	b .Lmain_L26
.Lmain_L18:
.Lmain_L19:
.Lmain_L20:
.Lmain_L21:
	str wzr,[sp,#76]
.Lmain_L23:
	add x10,sp,#76
	ldr w0,[x10]
	cmp w0,#2
	cset w0,lt
	cmp w0,#0
	b.ne .Lmain_L24
	b .Lmain_L25
.Lmain_L24:
	add x10,sp,#76
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#76
	str w0,[x10]
	b .Lmain_L23
.Lmain_L25:
	add x10,sp,#68
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#68
	str w0,[x10]
	add x10,sp,#36
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#36
	str w0,[x10]
	b .Lmain_L11
.Lmain_L26:
	cmp,#0
	cset w0,ne
	cmp w0,#0
	b.ne .Lmain_L29
	b .Lmain_L34
.Lmain_L29:
	cmp,#0
	cset w0,ne
	cmp w0,#0
	b.ne .Lmain_L32
	b .Lmain_L33
.Lmain_L32:
.Lmain_L33:
.Lmain_L34:
	add x10,sp,#60
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#60
	str w0,[x10]
	b .Lmain_L8
.Lmain_L35:
	add x10,sp,#52
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#52
	str w0,[x10]
	b .Lmain_L5
.Lmain_L36:
	add x10,sp,#44
	ldr w0,[x10]
	add w0,w0,#1
	add x10,sp,#44
	str w0,[x10]
	b .Lmain_L2
.Lmain_L37:
	add x10,sp,#36
	ldr w0,[x10]
	add x10,sp,#28
	str w0,[x10]
	add x10,sp,#28
	ldr w0,[x10]
	ldr w0,[sp,#28]
	add sp,sp,#96
	ret

