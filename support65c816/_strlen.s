	.65c816
	.a16
	.i16

	.export _strlen

_strlen:
	ldx	0,y
	lda	#0
	sta	@tmp
next:
	sep	#0x20
	.a8
	lda	0,x
	rep	#0x20
	.a16
	beq	done
	inc	@tmp
	inx
	bra	next
done:	lda	@tmp
	iny
	iny
	rts
