	.export __xplusplusuc
	.export __xplusplusc

	.code

__xplusplusc:
__xplusplusuc:
	ldaa	,x
	psha
	aba
	ldab	,x
	pulb
	rts
