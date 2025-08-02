;
;	a >= @tmp unsigned
;
	.export	__gteqtmpuc
	.export	__l_gteqtmpuc

	.code

__l_gteqtmpuc:
	jsr	__ytmpc
__gteqtmpuc:
	ldx	#0
	cmp	@tmp
	bcs	true
	txa
	rts
true:
	lda	#1
	rts
