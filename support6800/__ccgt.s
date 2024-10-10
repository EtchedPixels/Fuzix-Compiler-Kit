;
;	Compare D with TOS
;	TOS > D
;
	.export __ccgt
	.export __ccgtu

__ccgtu:
	tsx
	suba	2,x
	blo	true
	beq	cclow
	bra	false
__ccgt:
	tsx
	suba	2,x
	blt	true
	bne	false
cclow:
	subb	3,x
	blo	true
false:
	jmp	__false2
true:
	jmp	__true2
