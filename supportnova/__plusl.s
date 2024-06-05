;
;	32bit add of TOS and hireg:1
;
	.export __plusl
	.code
__plusl:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	3,0,2
	lda	2,-1,2
	dsz	__sp,0
	dsz	__sp,0
	lda	0,__hireg,0
	addz	3,1,szc
	inc	0,0
	add	2,0
	sta	0,__hireg,0
	lda	3,__fp,0
	jmp	@__tmp
