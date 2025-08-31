;
;	Compare A with @tmp
;
	.export __eqeqtmpc
	.export __eqeqtmpuc

__eqeqtmpc:
__eqeqtmpuc:
	ldx #0
	cmp @tmp
	bne false
true:	lda #1
	rts
false:
	txa
	rts


