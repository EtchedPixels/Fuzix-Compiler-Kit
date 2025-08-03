;
;	32bit boolean not
;
	.export __notl

__notl:
	stx	@tmp
	ldx	#0
	ora	@tmp
	ora	@hireg
	ora	@hireg+1
	bne	false
	; Was zero so set to one
	lda	#1
	rts
false:	txa
	rts
