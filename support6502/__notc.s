;
;	Turn A into 0 or 1 and flags
;

	.export __notc
__notc:
	ldx #0
	cmp #0
	beq setit
	txa
	rts
setit:
	lda #1
	rts
