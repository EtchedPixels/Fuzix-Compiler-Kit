;
;	Compare Y:D with TOS
;
	.export __ccnel

__ccnel:	
	tsx
	subb	5,x
	bne	true
	suba	4,x
	bne	true
	ldab	@hireg+1
	subb	3,x
	bne	true
	ldaa	@hireg
	suba	2,x
	bne	true
	jmp	__false4
true:	jmp	__true4
