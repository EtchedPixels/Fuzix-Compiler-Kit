	.export __xplusplusuc
	.export __xplusplusc

	.code
	.setcpu 6803

__xplusplusc:
__xplusplusuc:
	ldaa	,x
	psha
	aba
	staa	,x
	pulb
	rts
