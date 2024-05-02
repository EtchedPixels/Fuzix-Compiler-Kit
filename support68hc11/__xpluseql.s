;
;	Messy need to think how to do it better
;
	.export __xpluseql
	.export __xplusequl

	.code
__xpluseql:
__xplusequl:
	; add Y,D to ,X
	addd 2,x
	std 2,x
	bcc no_overflow
	iny
no_overflow:
	xgdy
	addd 0,x
	std 0,x
	xgdy
	rts
