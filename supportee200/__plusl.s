	.setcpu 4
	.export __plusl

	.code

__plusl:
	; top of stack plus hireg:b
	lda	6(s)	; 2 is saved X (junk), 4 is high 6 is low
	aab
	lda	4(s)	; high half
	stb	4(s)	; use as a save for the low word
	bnl	nocarry
	ina
nocarry:
	ldb	(__hireg)
	aab
	stb	(__hireg)
	ldb	4(s)	; get the low half back
	; now throw 4 bytes
	lda	4
	add	a,s
	rsr

