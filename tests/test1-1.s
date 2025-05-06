	.arch armv8-a
	.file	"test1-1.c"
	.text
	.global	r
	.data
	.align	2
	.type	r, %object
	.size	r, 4
r:
	.word	15
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB0:
	.cfi_startproc
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	mov	w0, 2
	str	w0, [sp, 8]
	mov	w0, 3
	str	w0, [sp, 12]
	ldr	w1, [sp, 8]
	ldr	w0, [sp, 12]
	add	w0, w1, w0
	add	w0, w0, 2
	add	sp, sp, 16
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
