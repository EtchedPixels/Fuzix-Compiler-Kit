	.export __xplusplusuc
	.export __xplusplusc
	.code

__xplusplusc:
__xplusplusuc:
	stb	,-s
	addb	,x
	stb	,x
	puls	b,pc
	