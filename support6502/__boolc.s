;
;	Turn A into 0 or 1 and flags
;
	.export __boolc

__boolc:
	ldx #0
	cmp #0
	bne setit
	rts
setit:
	lda #1
	rts
