;
;	xa < @tmp signed
;
	.export __ccgtu
	.export	__lttmp

	.code

__ccgtu:
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
