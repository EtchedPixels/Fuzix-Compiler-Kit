;
;	Compare XA with __tmp
;
	.export __netmp
	.export __netmpu
	.export __ccne

__ccne:
	jsr __poptmp
__netmp:
__netmpu:
	cmp @tmp
	bne true
	txa
	ldx #0
	cmp @tmp+1
	bne true2
	txa
	rts
true:	ldx #0
true2: lda #1
	rts
