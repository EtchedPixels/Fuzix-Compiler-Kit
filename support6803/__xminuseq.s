;
;	,X -= D
;
	.export __xminuseq
	.export __xminusequ

	.setcpu 6803

__xminuseq:
__xminusequ:
	std @tmp
	ldd ,x
	subd @tmp
	std ,x
	rts
