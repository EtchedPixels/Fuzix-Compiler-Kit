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
	beq 	swapit
	; Might be worth optimising >8 shift in two parts but unclear
	tax
	lda	@tmp
loop:
	lsr	a
	dex
	bne	loop
done:
	rts
swapit:
	lda	@tmp
	swa
	rts
