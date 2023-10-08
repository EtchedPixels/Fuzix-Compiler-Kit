	.65c816
	.a16
	.i16

	.export __shll

__shll:
	; shift TOS left A
	and #31
	bne shifts
	plx
	pla
	sta @tmp
	pla
	bra done
shifts:
	cmp #16
	bcs justshift
	; carry is clear so do 15 for 16
	sbc #15
	sta @count
	plx
	pla	; word we want
shifthi:
	asl a
	dec @count
	bne shifthi
	sta @tmp
	pla	; old lost high word
	stz @hireg
	lda @tmp
	phx
	rts
justshift:
	sta @count
	plx
	pla
	sta @tmp
	pla
shift:
	asl @tmp
	rol a
	dec @count
	bne shift
done:
	sta @hireg
	lda @tmp
	phx
	rts
	
