;
;	Compare D with TOS
;
	.export __cceq

__cceq:
	tsx
	subb	3,x
	bne	false
	suba	2,x
	bne	false
	jmp	__true2
false:	jmp	__false2
