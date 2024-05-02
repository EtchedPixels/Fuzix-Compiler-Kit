	.export __xplusplusuc
	.export __xplusplusc
	.code

__xplusplusc:
__xplusplusuc:
	pshb
	addb	,x
	stab	,x
	pulb
	rts
	