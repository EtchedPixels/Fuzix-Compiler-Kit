;
;	Preserve original value as result
;	Make re-entrant ?
;
	.export __xplusplusl
	.export __xplusplusul

	.code
__xplusplusl:
__xplusplusul:
	std	@tmp
	ldd	2,x
	pshb
	psha
	addd	@tmp
	std	2,x
	xgdy
	std	@tmp
	ldd	,x
	pshb
	psha
	adcb	@tmp+1
	adca	@tmp
	std	,x
	puly
	pula
	pulb
	rts
