	.setcpu	4
	.export	__switchl

	.code

__switchl:
	xfr	z,a
	sta	(-s)
	xfr	y,a
	sta	(-s)
	lda	(x+)	; count
	bz	found
	xay
	lda	(__hireg)
	xaz
swent:
	lda	(x+)
	sub	z,a
	bnz	next
	lda	(x+)
	sub	b,a
	bz	found
notfound:
	inx
	inx
	dcr	y
	bnz	swent
found:
	ldx	(x+)
	lda	(s+)
	xay
	lda	(s+)
	xaz
	jmp	(x)
next:
	inx
	inx
	bra	notfound
