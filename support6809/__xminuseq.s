;
;	,X -= D
;
	.export __minuseq

__minuseq:
	std ,--s
	ldd ,x
	subd ,s++
	std ,x
	rts
