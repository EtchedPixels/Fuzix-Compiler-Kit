	.export __xplusplusu
	.export __xplusplus
	.code

__xplusplus:
__xplusplusu:
	std	@tmp
	ldd	,x
	pshb
	psha
	addd	@tmp
	std	,x
	pula
	pulb
	rts
