;
;	,X -= D
;
	.export __xminuseq

__xminuseq:
	std ,--s
	ldd ,x
	subd ,s++
	std ,x
	rts
