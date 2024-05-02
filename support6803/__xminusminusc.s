;
;	,X - D
;
	.export __xmminusc
	.export __xmminusuc
	.code

__xmminusc:
__xmminusuc:
	ldaa ,x
	sba
	staa ,x
	tab
	rts
