	.65c816
	.a16
	.i16

	.export __gtxul

__gtxul:
	; shift hireg:a by x
	pha
	txa
	and #31
	beq nowork_out
	cmp #16
	bcc shifts
	sec
	sbc #16
	beq done_1
	tax
	pla
shift_1:
	asl a
	dex
	bne shift_1
	stz @hireg
	rts

done_1:
	pla
	lda @hireg
	stz @hireg
	rts

shifts:
	tax
	pla
shift_2:
	lsr @hireg
	ror a
	dex
	bne shift_2
	rts

nowork_out:
	pla
	rts
