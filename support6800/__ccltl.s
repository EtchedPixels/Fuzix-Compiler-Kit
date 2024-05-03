;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccltl
	.export __ccltul

__ccltul:
	tsx
	staa	@tmp
	stab 	@tmp+1
	ldaa	@hireg
	ldab	@hireg+1
	suba	2,x
	bhi	true
	beq	cclow
	bra	false
__ccltl:
	tsx
	staa	@tmp
	stab 	@tmp+1
	ldaa	@hireg
	ldab	@hireg+1
	suba	2,x
	bgt	true
	bne	false
cclow:
	subb	3,x
	bhi	true
	ldaa	@tmp
	ldab	@tmp+1
	suba	4,x
	bhi	true
	subb	5,x
	bhi	true
false:
	jmp	__falss4
true:
	jmp	__true4
