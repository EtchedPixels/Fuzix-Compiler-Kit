	.setcpu 4
	.export __div
	.export __rem

	.code

__div:
	stx	(-s)
	clr	x
	ori	b,b
	bp	plusdiv
	ivr	b
	inr	b
	inx		; count sign switches
plusdiv:
	lda	4(s)
	bp	plusdiv2
	iva
	ina
	dcx		; non zero means negate result
plusdiv2:
	jsr	__div16x16
	xab
signfix:
	ori	x,x
	bz	sign_good
	ivr	b
	inr	b
sign_good:
	ldx	(s+)
	inr	s
	inr	s
	rsr

__rem:
	stx	(-s)
	clr	x
	ori	a,a
	bp	plusmod
	ivr	b
	inr	b
	inx
plusmod:
	lda	4(s)
	bp	plusmod2
	iva	
	ina
plusmod2:
	jsr	__div16x16
	bra	signfix
