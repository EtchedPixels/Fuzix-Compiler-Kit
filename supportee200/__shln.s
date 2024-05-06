;
;	Shift left TOS by non constant B
;
	.setcpu	4
	.export __shl

	.code

__shl:
	lda	15
	nab
	bz	nowork
	lda	2(s)
	bz	nowork
	; Might be worth byte swap optimzations ?
next:
	sla
	dcr	b
	bnz	next
	inr	s
	inr	s	; pull TOS out of the way
	xab
	rsr
nowork:
	ldb	2(s)
	inr	s
	inr	s
	rsr
	