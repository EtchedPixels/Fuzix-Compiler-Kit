;
;	,X - D
;
	.export __xmminus
	.export __xmminusu
	.code

__xmminus:
__xmminusu:
	xgdy
	ldd ,x
	pshb
	psha
	xgdy
	tsy
	subd ,y
	std ,x
	ins
	ins
	pula
	pulb
	rts

