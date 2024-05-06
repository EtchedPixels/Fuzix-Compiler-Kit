;
;	Shift left (TOS) by non constant B
;
	.setcpu	4
	.export __shleqc

	.code

__shleqc:
	lda	15
	nab
	bz	nowork
	lda	2(s)
	ldab	(a)
	bz	nowork
	; Might be worth byte swap optimzations ?
next:
	slab
	dcr	b
	bnz	next
	xab
	lda	2(s)
	stbb	(a)
	inr	s
	inr	s	; pull TOS out of the way
	rsr
nowork:
	lda	2(s)
	ldbb	(a)
	inr	s
	inr	s	; pull TOS out of the way
	rsr
	