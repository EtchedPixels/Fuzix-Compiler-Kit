	.65c816
	.a16
	.i16

	.export __switchc
	.export __switchcu

__switchcu:
__switchc:
	; X holds the switch table A the value
	lda 0,x
	sta @tmp	; length
next:
	rep #0x20
	cmp 2,x
	sep #0x20
	beq gotswitch
	inx
	inx
	inx
	dec @tmp
	bne next
	; Fall through for default
	lda 2,x
	dec a
	pha
	rts
gotswitch:
	lda 3,x
	dec a
	pha
	rts
