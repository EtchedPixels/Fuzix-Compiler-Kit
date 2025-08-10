;
;	(TOS) | hireg:XA
;
	.export __oreql

__oreql:
	jsr	__poptmp
	; @tmp is now the pointer, Y is 0, pointer has been pulled off stack
	ora	(@tmp),y
	sta	(@tmp),y
	pha
	txa
	iny
	ora	(@tmp),y
	sta	(@tmp),y
	tax
	iny
	lda	@hireg
	ora	(@tmp),y
	sta	(@tmp),y
	sta	@hireg
	iny
	lda	@hireg+1
	ora	(@tmp),y
	sta	(@tmp),y
	sta	@hireg+1
	pla
	rts

	