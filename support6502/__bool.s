;
;	Turn XA into 0 or 1 and flags
;
	.export __bool

__bool:
	stx @tmp
	ldx #0
	ora @tmp
	bne setit
	rts
setit:
	lda #1
	rts
