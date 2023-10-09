	.65c816
	.a16
	.i16

	.export __ltxl

__ltxl:
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
	bra out_1
done_1:
	pla
out_1:
	sta @hireg
	lda #0
	rts

shifts:
	tax
	pla
shift_2:
	asl a
	rol @hireg
	dex
	bne shift_2
	rts

nowork_out:
	pla
	rts
