	.65c816
	.a16
	.i16

	.export __lsx
	.export __lsxu

__lsx:
__lsxu:
	sta	@tmp
	txa
	and	#15
	beq	done
	cmp	#8
	bcc	loop
	sbc	#7	; we branch to loop end
	tax
	lda	@tmp
	swa
	and	#0xFF00
	; carry is already set
	bra	next
loop:
	asl	a
next:
	dex
	bne	loop
	rts
done:	lda	@tmp
	rts
