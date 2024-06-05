;
;	32bit TOS minus hireg:1
;
	.export f__minusl
	.code
f__minusl:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	3,0,2
	lda	2,-1,2
	dsz	__sp,0
	dsz	__sp,0
	lda	0,__hireg,0
	subz	3,1,szc
	sub	2,0,skp
	adc	2,0
	sta	0,__hireg,0
	lda	3,__fp,0
	jmp	@__tmp
