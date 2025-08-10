;
;	(TOS) & hireg:XA
;
	.export __andeql

__andeql:
	jsr	__poptmp
	; @tmp is now the pointer, Y is 0, pointer has been pulled off stack
	and	(@tmp),y
	sta	(@tmp),y
	pha
	txa
	iny
	and	(@tmp),y
	sta	(@tmp),y
	tax
	iny
	lda	@hireg
	and	(@tmp),y
	sta	(@tmp),y
	sta	@hireg
	iny
	lda	@hireg+1
	and	(@tmp),y
	sta	(@tmp),y
	sta	@hireg+1
	pla
	rts

	