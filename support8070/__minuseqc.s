	.export __minuseqc

;	(TOS.B) += A

__minuseqc:
	pop	p2
	pop	p3
	push	p2
	st	a,:__tmp
	ld	a,0,p3
	sub	a,:__tmp
	st	a,0,p3
	ret
