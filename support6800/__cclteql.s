;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __cclteql
	.export __ccltequl

__ccltequl:
	tsx
	staa	@tmp
	stab	@tmp+1
	ldaa	@hireg
	ldab	@hireg+1
	suba	2,x
	blo	false
	bhi	true
	bra	cclow
__cclteql:
	tsx
	staa	@tmp
	stab	@tmp+1
	ldaa	@hireg
	ldab	@hireg+1
	suba	2,x
	blt	false
	bgt	true
cclow:
	subb	3,x
	blo	false
	bhi	true
	ldaa	@tmp
	suba	4,x
	blo	false
	bhi	true
	ldab	@tmp+1
	subb	5,x
	blo	false
true:
	jmp	__true4
false:
	jmp	__false4