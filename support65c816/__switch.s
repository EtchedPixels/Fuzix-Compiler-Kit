	.65c816
	.a16
	.i16

	.export __switch
	.export __switchu

__switchu:
__switch:
	; X holds the switch table A the value
	pha
	lda 0,x
	sta @tmp	; length
	pla
next:
	cmp 2,x
	beq gotswitch
	inx
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
	lda 4,x
	dec a
	pha
	rts

