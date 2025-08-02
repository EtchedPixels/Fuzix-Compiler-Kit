;
;	a < @tmp signed
;
	.export	__lttmpc
	.export	__l_lttmpc

	.code

__l_lttmpc:
	jsr	__ytmpc
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
