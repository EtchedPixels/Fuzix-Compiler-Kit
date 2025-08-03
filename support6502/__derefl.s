;
;	Load hireg:xa from (XA)
;
	.export __derefl

__derefl:
	sta	@tmp
	stx	@tmp+1
	ldy	#3
	lda	(@tmp),y
	sta	@hireg+1
	dey
	lda	(@tmp),y
	sta	@hireg
	dey
	lda	(@tmp),y
	tax
	dey
	lda	(@tmp),y
	rts

