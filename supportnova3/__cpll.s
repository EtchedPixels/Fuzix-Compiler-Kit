	.export __cpll
	.code

__cpll:	sta	3,__tmp,0
	lda	0,__hireg,0
	com	0,0
	com	1,1
	sta	0,__hireg,0
	mffp	3
	jmp	@__tmp,0
