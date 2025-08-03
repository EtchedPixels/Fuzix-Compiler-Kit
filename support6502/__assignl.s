;
;	(TOS) = hireg:XA
;
	.export __assignl

__assignl:
	jsr	__poptmp
	; @tmp is now the pointer
	; Y is 0
	pha
	sta	(@tmp),y
	iny
	txa
	sta	(@tmp),y
	iny
	lda	@hireg
	sta	(@tmp),y
	iny
	lda	@hireg+1
	sta	(@tmp),y
	pla
	rts
