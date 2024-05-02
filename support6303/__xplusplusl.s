;
;	Preserve original value as result
;	Make re-entrant ?
;
	.export __xplusplusl
	.export __xplusplusul

	.code
	.setcpu 6803

__xplusplusl:
__xplusplusul:
	std	@tmp
	ldd	2,x
	pshb
	psha
	addd	@tmp
	std	2,x
	ldd	,x
	pshb
	psha
	adcb	@hireg+1
	adca	@hireg
	std	,x
	pula
	pulb
	std	@hireg
	pula
	pulb
	rts
