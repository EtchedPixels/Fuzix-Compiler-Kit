;
;	Shift left (TOS) by non constant B
;
	.setcpu	4
	.export __shleql

	.code

__shleql:
	stx	(-s)
	xfr	y,a
	sta	(-s)
	ldx	6(s)
	lda	15
	nab
	bz	nowork
	xay
	lda	(x)
	ldb	2(x)
next:
	slr	b
	rlr	a
	dcr	y
	bnz	next
	sta	(x)
	stb	2(x)
out:
	sta	(__hireg)
	lda	(s+)
	xay
	ldx	(s+)
	inr	s
	inr	s	; pull TOS out of the way
	rsr
nowork:
	lda	(x)
	ldb	2(x)
	bra	out
