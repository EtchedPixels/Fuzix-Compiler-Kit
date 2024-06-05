;
;	NOVA stack handling support
;
;	SP is at 020 (autoinc indirect through so push is faster as we
;	push more than pop)
;
;	We don't emulate SAV and RET as we don't really use all their
;	features or need them.
;
;
;	Inline
;	mffpX:	lda	X,fp,0
;	mtfpX:	sta	X,fp,0
;	mfspX:	lda	X,sp,0
;	mtspX:	sta	X,sp,0
;
;	pshaX:	sta	X,@sp,0
;	popaX:	dsz	sp,0 dsz sp,0 lda X,@sp,0
;	(for simple cleanup we can avoid the pop stuff and just dsz it)
;	Leaves us needing to watch the sp gp by a word on interrupt/signal
;	for 2/3 we can and many helpers we can go via reg instead
;	popa3:	lda	3,sp,0	lda 3,0,3, dsz sp,0
;
;
;
;	called with
;	mov	3,2
;	jsr	@__enter,0
;	.word	framesize
;
f__enter:
	lda	1,0,3		; frame size
	inc	3,3
	sta	3,__tmp,0
	lda	0,__fp,0
	sta	2,@__sp,0	; stack return address
	sta	0,@__sp,0	; stack previous frame pointer
	lda	0,__sp,0
	sta	0,__fp,0	; set new frame pointer
	add	1,0		; adjust sp for new frame size
	sta	0,sp,0		; store new sp
	jmp	__tmp,0		; back to caller

;
;	jmp	@ret
;
ret:
	lda	2,__fp,0	; frame that was saved
	lda	1,0,2		; get the old fp
	sta	1,__fp,0	; restore it
	lda	1,-1,2		; get the return address
	sub	0,0
	inczl	0,0		; get value 2
	sub	0,2
	sta	2,__sp,0	; store stack pointer
	sta	1,__tmp,0
	jmp	@__tmp,0
