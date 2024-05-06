;
;	32bit multiply
;
;
;	TOS * hireg:B
;
	.setcpu	4
	.export __mull
	.export __mullu

	.code

__mull:
__mullu:
	stx	(-s)
	xfr	y,x
	stx	(-s)
	xfr	z,x
	stx	(-s)

	lda	(__hireg)
	xay
	xfr	b,z

	; Y:Z is one half

	lda	8(s)
	ldb	10(s)

	; A:B the other

	ldx	32	; counter

nextbit:
	slr	z
	rlr	y
	slr	b
	rlr	a
	bnl	noadd
	; Y:Z += A:B
	add	a,y
	add	b,z
	; Deal with carry by hand
	bnl	noadd
	inr	y
noadd:
	dcx
	bnz	nextbit
	; Result is now in Y:Z
	xfr	y,a
	sta	(__hireg)
	xfr	z,b
	lda	(s+)
	xay
	lda	(s+)
	xaz
	ldx	(s+)
	lda	4
	add	a,s	; Clean up caller frame
	rsr

