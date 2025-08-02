;
;	a < @tmp unsigned
;
	.export	__lttmpuc
	.export	__l_lttmpuc

	.code

__l_lttmpuc:
	jsr	__ytmpc
__lttmpuc:
	ldx	#0
	cmp	@tmp
	bcc	true
	txa
	rts
true:
	lda	#1
	rts
