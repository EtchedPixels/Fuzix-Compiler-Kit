;
;	(TOS) ^ hireg:XA
;
	.export __xoreql

__xoreql:
	jsr	__poptmp
	; @tmp is now the pointer, Y is 0, pointer has been pulled off stack
	eor	(@tmp),y
	sta	(@tmp),y
	pha
	txa
	iny
	eor	(@tmp),y
	sta	(@tmp),y
	tax
	iny
	lda	@hireg
	eor	(@tmp),y
	sta	(@tmp),y
	sta	@hireg
	iny
	lda	@hireg+1
	eor	(@tmp),y
	sta	(@tmp),y
	sta	@hireg+1
	pla
	rts

	