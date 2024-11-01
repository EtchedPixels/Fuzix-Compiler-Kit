;
;	Compare Y:D with TOS
;	TOS < Y:D
;
	.export __ccgtl
	.export __ccgtul

__ccgtul:
	tsx
	staa	@tmp
	stab	@tmp+1
	ldaa	@hireg
	ldab	@hireg+1
	suba	2,x
	blo	true
	beq	cclow
	bra	false
__ccgtl:
	tsx
	staa	@tmp
	stab	@tmp+1
	ldaa	@hireg
	ldab	@hireg+1
	suba	2,x
	blt	true
	bne	false
cclow:
	subb	3,x
	blo	true
	bne	false
	ldaa	@tmp
	ldab	@tmp+1
	suba	4,x
	blo	true
	bne	false
	subb	5,x
	blo	true
false:
	jmp	__false4
true:
	jmp	__true4
