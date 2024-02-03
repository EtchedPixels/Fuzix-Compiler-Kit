	.65c816
	.a16
	.i16

	.export __shleqxc
	.export __shleqxcu

; TODO - inline this one to use 8bit shifting ?

__shleqxc:
__shleqxcu:
	; Shift [A] X times
	stx @tmp
	tax
	rep #0x20
	lda 0,x
	sep #0x20
	ldx @tmp
	phx
	jsr __lsx
	plx
	rep #0x20
	sta 0,x
	sep #0x20
	and #0xFF
	rts

