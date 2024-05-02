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
	ldd 2,x
	subd @tmp
	std 2,x
	addd @tmp
	rts
