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
	leay 1,y
no_overflow:
	exg d,y
	addd 0,x
	std 0,x
	exg d,y
	rts
