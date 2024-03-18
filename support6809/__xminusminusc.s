;
;	,X - D
;
	.export __xmminusc
	.export __xmminusuc
	.code

__xmminusc:
__xmminusuc:
	stb ,-s
	ldb ,x
	tfr b,a
	suba ,s+
	sta ,x
	rts
