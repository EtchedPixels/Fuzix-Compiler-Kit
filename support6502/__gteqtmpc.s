;
;	a >= @tmp signed
;
	.export	__gteqtmpc

	.code

__gteqtmpc:
	ldx	#0
	sec
	sbc	@tmp
	bvc	l1
	eor	#$80
l1:
	bpl	true
	txa
	rts
true:
	lda	#1
	rts
