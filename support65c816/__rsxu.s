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
	sbc	#7	; We branch to loop end
	tax
	lda	@tmp
	swa
	and	#0xFF
	bra	next
loop:
	lsr	a
next:
	dex
	bne	loop
	rts
done:
	lda	@tmp
	rts
