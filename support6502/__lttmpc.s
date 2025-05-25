;
;	a < @tmp signed
;
	.export	__lttmpc

	.code

__lttmpc:
	ldx	#0
	sec
	sbc	@tmp
	bvc	l1
	eor	#$80
l1:
	bmi	true
	txa
	rts
true:
	lda	#1
	rts
