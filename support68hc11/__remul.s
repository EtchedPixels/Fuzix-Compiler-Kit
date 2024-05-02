	.export __remul
	.code
__remul:
	pshb
	psha
	pshy
	tsx
	jsr	div32x32
	; Result is in tmp2/tmp3
	ldy	@tmp2
	ldd	@tmp3
	pulx
	pulx
	pulx
	ins
	ins
	ins
	ins
	jmp	,x
