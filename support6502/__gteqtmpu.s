;
;	xa >= @tmp unsigned
;
	.export	__gteqtmpu

	.code

__gteqtmpu:
	cmp	@tmp
	txa
	ldx	#0
	sbc	@tmp+1
	bcs	true
	txa
	rts
true:
	lda	#1
	rts
