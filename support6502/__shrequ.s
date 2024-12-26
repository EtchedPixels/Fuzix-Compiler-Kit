;
;	(TOS) >> A
;
	.export __shrequ
	.export __shreq
;

__shrequ:
	jsr	__poptmp
	; Y is set to 0
	and	#15
	tax
	beq	out
	lda	(@tmp),y
	sta	@tmp1
	iny
	lda	(@tmp),y
do_pve:
	sta	@tmp1+1
loop:
	lsr	@tmp1+1
	ror	@tmp1
	dex
	bne	loop
done:
	lda	@tmp1+1
	sta	(@tmp),y
	tax
	dey
	lda	@tmp1
	sta	(@tmp),y
	rts

out:
	iny
	lda	(@tmp),y
	tax
	dey
	lda	(@tmp),y
	rts


__shreq:
	jsr	__poptmp
	and	#15
	tax
	beq	out
	lda	(@tmp),y
	sta	@tmp1
	iny
	lda	(@tmp),y
	bpl	do_pve
	sta	@tmp1+1
nloop:
	sec
	ror	@tmp1+1
	ror	@tmp1
	dex
	bne	nloop
	jmp	done
