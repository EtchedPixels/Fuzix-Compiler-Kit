	.65c816
	.a16
	.i16

	.export __rsx

__rsx:
	sta	@tmp
	txa
	and	#15
	beq	done
	tax
	lda	@tmp
	bpl	rsxn
loop:
	lsr	a
	dex
	bne	loop
done:
	rts

rsxn:
	sec
	ror	a
	dex
	bne	rsxn
	rts

