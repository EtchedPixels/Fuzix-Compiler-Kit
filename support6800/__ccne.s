;
;	Compare D with TOS
;
	.export __ccne

__ccne:	
	tsx
	subb	3,x
	bne	true
	suba	2,x
	bne	true
	jmp	__false2
true:	jmp	__true2
