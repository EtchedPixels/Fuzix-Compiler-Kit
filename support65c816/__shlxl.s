	.65c816
	.a16
	.i16

	.export __shlxl
	.export __shlxul
	.export __shleqxl
	.export __shleqxul

	; shift hireg:A right by X (signed version)

__shlxl:
__shlxul:
	sta @tmp
	txa
	and #31
	beq nowork
	cmp #16
	bcc shiftit
	sec
	sbc #16
	bne shift16
	lda @tmp
	sta @hireg
	lda #0
	rts
shift16:
	tax
shift16l:
	asl a
	dex
	bne shift16l
	sta @hireg
	lda #0
	rts

nowork:
	lda @tmp
	rts

	; Need to do all the bits
shiftit:
	tax
	lda @tmp
shiftitl:
	asl a
	rol @hireg
	dex
	bne shiftitl
	rts

__shleqxul:
__shleqxl:
	; x is the pointer, A is the shift value
	sta @tmp
	lda 2,x
	sta @hireg
	lda 0,x
	phx
	ldx @tmp
	jsr __shlxl
	plx
	pha
	sta 0,x
	lda @hireg
	sta 2,x
	pla
	rts
