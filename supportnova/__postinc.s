	.export f__postinc
	.export f__postincl
	.code
; TODO Should now be obsolete

f__postinc:
	; TOS is addr, AC1 value
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	lda	0,0,2
	add	0,1
	sta	1,0,2
	mov	0,1
	lda	3,__fp,0
	jmp	@__tmp,0

f__postincl:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2
	lda	3,1,2
	lda	2,0,2
	lda	0,__hireg,0
	sta	2,__hireg,0
	addz	3,1,szc
	inc	0,0
	add	2,0
	lda	2,__sp,0
	lda	2,0,2
	dsz	__sp,0
	sta	0,0,2
	sta	1,1,2
	mov	3,1
	lda	3,__fp,0
	jmp	@__tmp,0

	