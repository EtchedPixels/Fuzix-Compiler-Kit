;
;	Shift right (TOS) by non constant B
;
	.setcpu	4
	.export __shreql

	.code

__shreql:
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
	sra
	rrr	b
	dcx
	bnz	next
	sta	(x)
	stb	2(x)
out:
	sta	(__hireg)
	lda	(s+)
	xay
	ldx	(s+)
	inr	s
	inr	s
	rsr
nowork:
	lda	(x)
	ldb	2(x)
	bra	out
	