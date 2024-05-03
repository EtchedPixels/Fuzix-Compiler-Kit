	.export __pop4
	.export __false4
	.export __true4
	.code

__false4:
	clra
	clrb
	bra __pop4
__true4:
	clra
	ldab	#1
__pop4:
	clra
	tsx
	ldx	,x
	ins
	ins
	ins
	ins
	ins
	ins
	tstb
	jmp	,x
