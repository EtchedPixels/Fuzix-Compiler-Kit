;
;	Compare D with TOS
;	TOS <= D
;
	.export __cclteq
	.export __ccltequ

__ccltequ:
	tsx
	suba	2,x
	blo	false
	bhi	true
	bra	cclow
__cclteq:
	tsx
	suba	2,x
	blt	false
	bgt	true
cclow:
	subb	3,x
	blo	false
true:
	jmp	__true2
false:
	jmp	__false2
