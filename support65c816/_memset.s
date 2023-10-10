	.65c816
	.a16
	.i16

	.export _memset

_memset:
	ldx	0,y		;src
	lda	2,y		;value
	lda	4,y
	sta	@tmp
nextbyte:
	sep	#0x20
	.a8
	sta	0,x
	rep	#0x20
	.a16
	inx
	dec	@tmp
	bne	nextbyte
	ldx	0,y
	jmp	__fnexit6
