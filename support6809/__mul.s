;
;	D = top of stack * D
;

	.export __mul

	.code

__mul:
	leas	4,s		; workspace
	std	,--s		; save value
	lda	9,s		; low byte
	mul			; D is now low x low
	std	2,s
	lda	8,s		; high byte
	ldb	,-s
	mul			; D is now high x low
	std	3,s
	lda	8,s		; low byte
	ldb	,-s		; high byte of D
	mul			; D is now low x high
	addd	2,s		; High bytes
	tfr	b,a		; Shift left 8, discarding
	clrb
	addd	0,s		; Add the low x low
	ldx	4,s		; return value
	leas	8,s		; clean stack
	jmp	,x		; home
