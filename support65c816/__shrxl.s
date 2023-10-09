	.65c816
	.a16
	.i16

	.export __shrxl
	.export __shrxul
	.export __shreqxl
	.export __shreqxul

	; shift hireg:A right by X (signed version)

__shrxl:
	sta @tmp
	lda @hireg
	bmi do_signed
	; High bit is 0, so use the unsigned shift path
__shrxul:
	sta @tmp
__shrxul1:
	txa
	and #31
	beq nowork
	cmp #16
	bcc shiftit
	sec
	sbc #16
	bne shift16
	lda @hireg
	stz @hireg
	rts
shift16:
	tax
	lda @hireg
	stz @hireg
shift16l:
	lsr a
	dex
	bne shift16l
	rts

nowork:
	lda @tmp
	rts

	; Need to do all the bits
shiftit:
	tax
	lda @tmp
shiftitl:
	lsr @hireg
	ror a
	dex
	bne shiftitl
	rts

do_signed:
	txa
	and #31
	beq nowork
	cmp #16
	bcc shiftits
	sec
	sbc #16
	bne shifts16
	lda @hireg
	stz @hireg
	dec @hireg
	rts
shifts16:
	tax
	lda #0xffff
	sta @hireg
	lda @tmp
shifts16l:	; shift 16bit leading 1 
	sec
	ror a
	dex
	bne shifts16l
	rts

	; Full 16 bit with leading 1
shiftits:
	tax
	lda @tmp
shiftitsl:
	sec
	ror @hireg
	ror a
	dex
	bne shiftitsl
	rts

; Could optimize these to shift the ,x fields in place ? */
__shreqxul:
	; x is the pointer, A is the shift value
	sta @tmp
	lda 2,x
	sta @hireg
	lda 0,x
	phx
	ldx @tmp
	jsr __shrxul
	plx
	pha
	sta 0,x
	lda @hireg
	sta 2,x
	pla
	rts

__shreqxl:
	; x is the pointer, A is the shift value
	sta @tmp
	lda 2,x
	sta @hireg
	lda 0,x
	phx
	ldx @tmp
	jsr __shrxl
	plx
	pha
	sta 0,x
	lda @hireg
	sta 2,x
	pla
	rts
