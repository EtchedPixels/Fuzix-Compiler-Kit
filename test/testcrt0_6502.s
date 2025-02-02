	.code ; (at 0x0200)

start:	ldx	#0xFF
	txs		; stack at 01FF
	lda	#0
	sta	sp
	lda	#0xFD	; user stack at FD00
	sta	sp+1
	jsr	_main
	; return and exit (value is in XA)
	sta	$FEFF
	; Write to FEFF terminates

