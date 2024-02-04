	.65c816
	.a16
	.i16

	.export __divxu
	.export __remxu
	.export __divx
	.export __remx	

__remxu:
	sta	@dividend
	stx	@divisor
	; Divide dividend by divisor leaving result in A and remainder in
	; @dividend
div16:
	ldx	#16
	lda	#0
divnext:
	asl	@dividend
	rol	a
	cmp	@divisor
	bcc	zerobit
	sbc	@divisor	; C already set
	inc	@dividend	; set the bottom bit
zerobit:
	dex
	bne	divnext
	rts

__divxu:
	jsr	__divxu
	lda	@dividend
	rts

__divx:
	stz	@sign		; needs to be word
	ora	#0
	bpl	nonegate
	inc	@sign
	eor	#0xFFFF
	inc	a
nonegate:
	sta	@dividend
	txa
	bpl	nonegate2
	inc	@sign
	eor	#0xFFFF
	inc	a
nonegate2:
	sta	@divisor
	jsr	div16
	tax
	lda	@sign
	and	#1
	beq	divpos
	txa
	eor	#0xFFFF
	inc	a
	rts
divpos:
	txa
	rts

__remx:
	stz	@sign		; needs to be word
	ora	#0
	bpl	rnonegate
	eor	#0xFFFF
	inc	a
rnonegate:
	sta	@dividend
	txa
	bpl	rnonegate2
	inc	@sign
	eor	#0xFFFF
	inc	a
rnonegate2:
	sta	@divisor
	jsr	div16
	lda	@sign
	and	#1
	beq	rempos
	lda	@dividend
	eor	#0xFFFF
	inc	a
	rts
rempos:
	lda	@dividend
	rts
