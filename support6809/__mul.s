;
;	D = top of stack * D
;
	.export __mul
	.export __reg_mul

	.code

__mul:
	std	,--s		; Save one half
	; We are now doing mul 0,s with 4,s
	lda	5,s		; low byte
	mul			; low x low
	std	,--s		; save low
	lda	7,s		; low byte of arg1
	ldb	2,s		; high byte of arg2
	mul
	addb	0,s		; add to upper half of result
	stb	0,s
	lda	6,s		; upper byte of arg1
	ldb	3,s		; lower byte of arg2
	mul
	addb	0,s		; add to upper half
	tfr	b,a
	ldb	1,s		; get into D
	ldx	4,s		; return address
	leas	8,s		; fix the stack
	jmp	,x

__regmul:			; U * D
	pshs	u
	jsr	__mul
	; Called func popped u
	tfr	d,u
	rts

