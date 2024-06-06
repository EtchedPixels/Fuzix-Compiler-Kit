;
;	,X - D
;
	.export __xmminusc
	.export __xmminusuc
	.code

__xmminusc:
__xmminusuc:
	ldaa ,x
	psha
	sba
	staa ,x
	pulb
	rts
