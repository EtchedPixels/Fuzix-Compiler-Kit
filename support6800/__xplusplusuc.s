	.export __xplusplusuc
	.export __xplusplusc

	.code

__xplusplusc:
__xplusplusuc:
	ldaa	,x
	psha
	aba
	staa	,x
	pulb
	rts
