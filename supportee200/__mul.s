	.setcpu 4
	.export __mul
	.export __mulu

	.code

__mul:
__mulu:
	; Bit shift multiply
	stx	(-s)
	xfr	y,x
	stx	(-s)

	clr	y
	ldb	6(s)
	ldx	16		; counter
	; Now do 16 iterations
nextbit:
	slr	y
	sla
	bnl	noadd
	add	b,y
noadd:	dcx
	bnz	nextbit

	; Result is now in Y
	xfr	y,b
	lda	(s+)
	xay
	ldx	(s+)
	inr	s
	inr	s
	rsr
