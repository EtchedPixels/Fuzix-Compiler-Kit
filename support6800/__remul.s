	.export __remul
	.code
__remul:
	pshb
	psha
	ldaa	@hireg
	ldab	@hireg+1
	pshb
	psha
	tsx
	jsr	div32x32
	; Result is in tmp2/tmp3
	ldaa	@tmp2
	ldab	@tmp2+1
	staa	@hireg
	stab	@hireg+1
	ldaa	@tmp3
	ldab	@tmp3+1
	ldx		4,x
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	jmp	0,x
