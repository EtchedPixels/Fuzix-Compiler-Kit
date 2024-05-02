;
;	Messy need to think how to do it better
;
	.export __xpluseql
	.export __xplusequl

	.code

	.setcpu 6803

__xpluseql:
__xplusequl:
	; add hireg:D to ,X
	addd 2,x
	std 2,x
	ldd @hireg
	adcb 1,x
	adca ,x
	std ,x
	std @hireg
	ldd 2,x
	rts
