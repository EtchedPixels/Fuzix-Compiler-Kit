;
;	Extended call return for recursions
;
	.export pcall
	.export pret

	.zp
pret:	.word	__pret
pcall:	.word	__pcall

	.code
__pcall:
	.word	0
	lda	@sp		; Adjust stack by two words
	add	=-2
	sta	@sp
	ldx	@sp		; Get a pointer
	lda	@fp
	sta	@1,1		; Save FP
	lda	__pcall
	sta	@0,1		; save old PC
	stx	@fp		; set new frame pointer
	jmp	*@0,1		; run subroutine

__pret:
	ima	@fp		; swap work and fp
	sta	@sp		; save fp to sp
	lda	*@sp		; get old fp
	ima	@fp		; swap old fp and work
	irs	@sp		; move up stack
	irs	*@sp		; return addr bump past .word
	jmp	*@sp		; and return

	; We use this instead to build the save table
	.commondata

	.word __pcall

