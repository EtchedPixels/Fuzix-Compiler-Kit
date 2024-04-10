;
;	,X -= D
;
	.export __xminuseq
	.export __xminusequ

__xminuseq:
__xminusequ:
	std ,--s
	ldd ,x
	subd ,s++
	std ,x
	rts
