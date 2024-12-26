;
;	(TOS) >> A
;
	.export __shrequc
	.export __shreqc
;

__shrequc:
	jsr	__poptmp
	and	#7
	tax
	beq	out
	lda	(@tmp),y
loop:
	lsr	a
	dex
	bne	loop
	sta	(@tmp),y
	rts
out:
	lda	(@tmp),y
	rts

__shreqc:
	jsr	__poptmp
	and	#7
	tax
	beq	out
	lda	(@tmp),y
	bpl	loop
nloop:
	sec
	ror	a
	dex
	bne	nloop
	sta	(@tmp),y
	rts
