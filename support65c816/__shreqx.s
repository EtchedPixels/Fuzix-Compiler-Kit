	.65c816
	.a16
	.i16

	.export __shreqx

__shreqx:
	; Shift [A] X times
	stx @tmp		; save shift count
	tax			; pointer into X
	lda 0,x			; value into A
	phx
	ldx @tmp		; shift count back into X
	jsr __rsx		; run shift helper
	plx			; pointer back into X
	sta 0,x			; store value
	rts

