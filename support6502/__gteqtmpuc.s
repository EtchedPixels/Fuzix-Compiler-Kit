;
;	a >= @tmp unsigned
;
	.export	__gteqtmpuc

	.code

__gteqtmpuc:
	ldx	#0
	cmp	@tmp
	bcs	true
	txa
	rts
true:
	lda	#1
	rts
