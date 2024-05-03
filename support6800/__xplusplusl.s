;
;	Preserve original value as result
;	Make re-entrant ?
;
	.export __xplusplusl
	.export __xplusplusul

	.code

__xplusplusl:
__xplusplusul:
	staa	@tmp
	stab	@tmp+1
	ldaa	2,x
	ldab	3,x
	pshb
	psha
	addb	@tmp+1
	adca	@tmp
	staa	2,x
	stab	3,x
	ldaa	,x
	ldab	1,x
	pshb
	psha
	adcb	@hireg+1
	adca	@hireg
	staa	,x
	stab	1,x
	pula
	pulb
	staa	@hireg
	stab	@hireg+1
	pula
	pulb
	rts
