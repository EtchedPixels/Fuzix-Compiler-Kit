;
;	Shift right (TOS) by non constant B unsigned
;
	.setcpu	4
	.export __shrequl

	.code

__shrequl:
	stx	(-s)
	xfr	y,a
	sta	(-s)
	ldx	4(s)
	lda	15
	nab
	bz	nowork
	xay
	lda	(x)
	ldb	2(x)
	; First shift by hand
	rl
	rrr	a
	bra	go
next:
	sra
go:
	rrr	b
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
	