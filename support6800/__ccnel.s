;
;	Compare Y:D with TOS
;
	.export __ccnel

__ccnel:	
	tsx
	subb	4,x
	bne	true
	suba	2,x
	bne	true
	ldab	@hireg+1
	subb	3,x
	ldaa	@hireg
	suba	3,x
	bne	true
	jmp	false4
true:	jmp	true4
