;
;	D = top of stack * D
;
	.export __mul

	.code
	.setcpu 6803

__mul:
	pshb
	psha			; Save one half
	tsx
	; We are now doing mul 0,x with 4,x
	ldaa	5,x		; low byte
	mul			; low x low
	pshb
	psha			; save low
	ldaa	7,x		; low byte of arg1
	ldab	2,x		; high byte of arg2
	mul
	addb	0,x		; add to upper half of result
	stab	0,x
	ldaa	6,x		; upper byte of arg1
	ldab	3,x		; lower byte of arg2
	mul
	addb	0,x		; add to upper half
	tba
	ldab	1,x		; get into D
	pulx			; discard stacked word
	pulx			; return
	inx
	inx
	inx
	inx
	jmp	,x
