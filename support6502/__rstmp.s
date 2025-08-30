;
;	@tmp >> XA (only A low bits matter)
;
;	TODO; consider checking 8+ shift and byteswap but not clear
;	it's worth it
;
	.export __rstmpu
	.export __rstmp
	.export __l_gtgt
	.export __l_gtgtu

	.code

__l_gtgt:
	sta	@tmp
	stx	@tmp+1
	dey
	lda	(@sp),y
__rstmp:
	ldx	@tmp+1
	bmi	__rsneg
	; Positive and unsigned
__rstmpu:
	and	#15
	beq	done
	tax
loop:	lsr	@tmp+1
	ror	@tmp
	dex
	bne	loop
done:
	lda	@tmp
	ldx	@tmp+1
	rts
	; Negative
__rsneg:
	and	#15
	beq	done
	tax
loopn:	sec
	ror	@tmp+1
	ror	@tmp
	dex
	bne	loopn
	lda	@tmp
	ldx	@tmp+1
	rts

__l_gtgtu:
	sta	@tmp
	stx	@tmp+1
	dey
	lda	(@sp),y
	jmp	__rstmpu
