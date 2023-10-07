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
	swa
	and	#0xFF00
	; carry is already set
	sbc	#8
	beq	done
	tax
	lda	@tmp
loop:
	asl	a
	dex
	bne	loop
done:	rts
