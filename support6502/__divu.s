	.export __divu
	.export __divtmpu
	.export __remu
	.export __remtmpu
	.export __dodivu
;
;	TOS / XA
;
__divu:
	jsr	__poptmp
	; Now @tmp/XA

;
;	Save the divisor into @tmp1
;
__divtmpu:
	sta	@tmp1
	stx	@tmp1+1

;
;	This is the standard rotate and subtract division. We also use it
;	for the signed divisions after sorting the signs out elsewhere
;
__dodivu:
	lda	#0
	sta	@tmp2
	sta	@tmp2+1
	ldy	#16

loop:
	asl	@tmp
	rol	@tmp+1
	rol	@tmp2
	rol	@tmp2+1

	lda	@tmp2
	sec
	sbc	@tmp1
	tax
	lda	@tmp2+1
	sbc	@tmp1+1
	bcc	skip
	stx	@tmp2
	sta	@tmp2+1
skip:	dey
	bne	loop
	; tmp2 holds the remainder tmp the division result
	ldx	@tmp+1
	lda	@tmp
	rts

__remu:
	jsr	__poptmp
__remtmpu:
	jsr	__divu
	ldx	@tmp2+1
	lda	@tmp2
	rts
