	.export __xplusplusuc
	.export __xplusplusc
	.code

__xplusplusc:
__xplusplusuc:
	pshs	b
	lda	,x
	tfr	a,b
	adda	,s+
	sta	,x
	rts

