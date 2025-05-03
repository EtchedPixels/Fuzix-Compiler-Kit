;
;	Compare D with TOS
;	TOS >= D
;
	.export __ccgteq
	.export __ccgtequ

__ccgtequ:
	tsx
	suba	2,x
	bhi	false
	blo	true
	bra	cclow
__ccgteq:
	tsx
	suba	2,x
	bgt	false
	blt	true
cclow:
	subb	3,x
	bhi	false
true:
	jmp	__true2
false:
	jmp	__false2
