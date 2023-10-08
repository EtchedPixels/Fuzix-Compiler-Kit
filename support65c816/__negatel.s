	.65c816
	.a16
	.i16

	.export __negatel

__negatel:
	eor #0xffff
	tax
	lda @hireg
	eor #0xffff
	sta @hireg
	txa
	inc a
	bne done
	inc @hireg
done:	rts
