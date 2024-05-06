	.setcpu	4
	.export	__switch

	.code

__switch:
	xfr	y,a
	sta	(-s)
	lda	(x+)	; count
	bz	found
	xay
swent:
	lda	(x+)
	sub	b,a
	bz	found
	inx
	inx
	dcr	y
	bnz	swent
found:
	ldx	(x+)
	lda	(s+)
	xay
	jmp	(x)


	
