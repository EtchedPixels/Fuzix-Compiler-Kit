;
;	Compare Y:D with TOS
;
	.export __cceql

__cceql:
	tsx
	subb	5,x
	bne	false
	suba	4,x
	bne	false
	ldab	@hireg+1
	subb	3,x
	bne	false
	ldaa	@hireg
	suba	2,x
	bne	false
	jmp	__true4
false:	jmp	__false4
