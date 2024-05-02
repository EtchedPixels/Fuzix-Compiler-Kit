;
;	,X -= D
;
	.export __xminuseq
	.export __xminusequ

__xminuseq:
__xminusequ:
	pshb
	psha
	tsy
	ldd ,x
	subd ,y
	std ,x
	pulx
	rts
