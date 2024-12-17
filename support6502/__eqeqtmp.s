;
;	Compare XA with __tmp
;
	.export __eqeqtmp
	.export __eqeqtmpu

__eqeqtmp:
__eqeqtmpu:
	cmp @tmp
	bne false
	txa
	ldx #0
	cmp @tmp+1
	bne false2
	lda #1
	rts
false:	ldx #0
false2: txa
	rts
