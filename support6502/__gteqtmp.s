;
;	xa >= @tmp signed
;
	.export __ccltequ
	.export	__gteqtmp

	.code

__ccltequ:
	jsr	__poptmp
__gteqtmp:
	cmp	@tmp
	txa
	ldx	#0
	sbc	@tmp+1
	bvc	l1
	eor	#$80
l1:
	bpl	true
	txa
	rts
true:
	lda	#1
	rts
