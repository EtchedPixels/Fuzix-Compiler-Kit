	.export f__pluseql
	.export f__cpluseql

f__pluseql:
	lda	0,__hireg,0
peq:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2
	lda	3,1,2
	lda	2,0,2
	; Now doing 0/1 + 2/3
	addz	3,1,szc
	inc	0,0
	add	2,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sta	0,0,2
	sta	1,1,2
	sta	0,__hireg,0
	lda	3,__fp,0
	jmp	@__tmp,0

f__cpluseql:
	inc	3,3
	inc	3,3
	sta	1,@__sp,0
	lda	0,-2,3
	lda	1,-1,3
	jmp	peq,1
