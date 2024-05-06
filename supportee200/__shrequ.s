;
;	Shift right TOS by non constant B
;
	.setcpu	4
	.export __shrequ

	.code

; Need to do a spot of work for the unsinged shift
__shrequ:
	lda	15
	nab
	bz	nowork
	lda	2(s)
	lda	(a)
	bz	nowork
	rl
	rrr	a
	bra	go
	; Might be worth byte swap optimzations ?
next:
	sra
go:
	dcr	b
	bnz	next
done:
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
	inr	s
	rsr
	