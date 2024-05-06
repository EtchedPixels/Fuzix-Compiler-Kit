;
;	Shift right TOS by non constant B
;
	.setcpu	4
	.export __shrl

	.code

__shrl:
	stx	(-s)
	ldx	4(s)
	lda	15
	nab
	bz	nowork
	xax
	lda	4(s)
	ldb	6(s)
next:
	sra
	rrr	b
	dcx
	bnz	next
out:
	sta	(__hireg)
	ldx	(s+)
	lda	4
	add	a,s	; pull TOS out of the way
	rsr
nowork:
	lda	4(s)
	ldb	6(s)
	bra	out
	