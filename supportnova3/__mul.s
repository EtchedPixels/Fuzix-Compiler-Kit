	.export __mul

	.code

__mul:
	subc	0,0
	sta	3,__tmp,0
	lda	3,N16,1
loop:	movr	1,1,snc
	movr	0,0,skp
	addzr	2,0
	inc	3,3,szr
	jmp	loop,1
	movcr	1,1
	mffp	3
	jmp	@__tmp,1
N16:	.word	16
