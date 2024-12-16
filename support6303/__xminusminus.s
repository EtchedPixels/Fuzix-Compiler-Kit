;
;	,X - D
;
	.export __xmminus
	.export __xmminusu
	.code

	.setcpu 6803

__xmminus:
__xmminusu:
	std @tmp
	ldd ,x
	subd @tmp
	std ,x
	addd @tmp
	rts
