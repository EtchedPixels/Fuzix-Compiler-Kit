;
;	Shift left (TOS) by non constant B
;
	.setcpu	4
	.export __shleq

	.code

__shleq:
	lda	15
	nab
	bz	nowork
	lda	2(s)
	lda	(a)
	bz	nowork
	; Might be worth byte swap optimzations ?
next:
	sla
	dcr	b
	bnz	next
	xab
	lda	2(s)
	stb	(a)
	inr	s
	inr	s	; pull TOS out of the way
	rsr
nowork:
	lda	2(s)
	ldb	(a)
	inr	s
	inr	s	; pull TOS out of the way
	rsr
	