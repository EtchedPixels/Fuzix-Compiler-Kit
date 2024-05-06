	.setcpu 4
	.export __booll
	.export __notl

	.code
__booll:
	lda	(__hireg)
	ori	a,b
	bz	ret0	; B already 0
	cla
	sta	(__hireg)
	ldb	1
	; ? does ldb set flags ?
ret0:
	rsr

__notl:
	lda	(__hireg)
	ori	a,b
	bz	ret1
	clr	b
	rsr
ret1:
	; was 0 now 1
	inr	b
	rsr
