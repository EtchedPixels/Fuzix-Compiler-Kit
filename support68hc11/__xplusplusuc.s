	.export __xplusplusuc
	.export __xplusplusc

	.code

__xplusplusc:
__xplusplusuc:
	tba
	ldab    ,x
	aba
	staa	,x
	rts
