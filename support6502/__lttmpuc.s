;
;	a < @tmp unsigned
;
	.export	__lttmpuc

	.code

__lttmpuc:
	ldx	#0
	cmp	@tmp
	bmi	true
	txa
	rts
true:
	lda	#1
	rts
