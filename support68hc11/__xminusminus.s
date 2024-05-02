;
;	,X - D
;
	.export __xmminus
	.export __xmminusu
	.code

__xmminus:
__xmminusu:
	std @tmp
	ldd ,x
	subd @tmp
	std ,x
	addd @tmp
	rts

