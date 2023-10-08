	.65c816
	.a16
	.i16

	.export __minusl

__minusl:
	; TOS - hireg:a
	plx
	sta @tmp
	sec
	pla		; low word
	sbc @tmp
	sta @tmp
	pla
	sbc @hireg
	sta @hireg
	lda @tmp
	phx
	rts
