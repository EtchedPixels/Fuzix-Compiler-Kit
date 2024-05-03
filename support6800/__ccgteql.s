;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgteql
	.export __ccgtequl

__ccgtequl:
	tsx
	staa	@tmp
	stab	@tmp+1
	ldaa	@hireg
	ldab	@hireg+1
	suba	2,x
	bhi	false
	blo	true
	bra	cclow
__ccgteql:
	tsx
	staa	@tmp
	stab	@tmp+1
	ldaa	@hireg
	ldab	@hireg+1
	suba	2,x
	bgt	false
cclow:
	subb	3,x
	bhi	false
	ldaa	@tmp
	ldab	@tmp+1
	suba	4,x
	bhi	false
	subb	5,x
	bhi	false
true:
	jmp	__true4
false:
	jmp	__false4
