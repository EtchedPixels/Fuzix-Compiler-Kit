	.export __xplusplusuc
	.export __xplusplusc

	.code
	.setcpu 6803

__xplusplusc:
__xplusplusuc:
	tba
	ldab    ,x
	aba
	staa	,x
	rts
