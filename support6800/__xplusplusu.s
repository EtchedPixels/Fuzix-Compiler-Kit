	.export __xplusplusu
	.export __xplusplus
	.code

__xplusplus:
__xplusplusu:
	staa	@tmp
	stab	@tmp+1
	ldaa	,x
	ldab	1,x
	pshb
	psha
	addb	@tmp+1
	adca	@tmp
	staa	,x
	stab	1,x
	pula
	pulb
	rts
