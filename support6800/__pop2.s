	.export __pop2
	.export __true2
	.export __false2
	.code

__false2:
	clra
	clrb
	bra __pop2
__true2:
	clra
	ldab	#1
__pop2:
	clra
	tsx
	ldx	,x
	ins
	ins
	ins
	ins
	tstb
	jmp	,x
