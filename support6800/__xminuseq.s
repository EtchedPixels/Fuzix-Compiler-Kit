;
;	,X -= D
;
	.export __xminuseq
	.export __xminusequ

__xminuseq:
__xminusequ:
	staa @tmp
	stab @tmp+1
	ldaa ,x
	ldab 1,x
	subb @tmp+1
	sbca @tmp
	staa ,x
	stab 1,x
	rts
