	.65c816
	.a16
	.i16

	.export __rsxu

__rsxu:
	sta	@tmp
	txa
	and	#15
	beq	done
	cmp	#8
	bcc	loop
	swa
	and	#0x00FF
	; carry is already set
	sbc	#8
	beq	done
	tax
	lda	@tmp
loop:
	lsr	a
	dex
	bne	loop
done:
	rts
