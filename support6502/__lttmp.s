;
;	xa < @tmp signed
;
	.export __ccgt
	.export	__lttmp
	.export	__l_lttmp

	.code

__l_lttmp:
	jsr	__ytmp
	jmp	__lttmp
__ccgt:
	jsr	__poptmp
__lttmp:
	cmp	@tmp
	txa
	ldx	#0
	sbc	@tmp+1
	bvc	l1
	eor	#$80
l1:
	bmi	true
	txa
	rts
true:
	lda	#1
	rts
