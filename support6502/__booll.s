;
;	32bit boolean not
;
	.export __booll

__booll:
	stx	@tmp
	ldx	#0
	ora	@tmp
	ora	@hireg
	ora	@hireg+1
	beq	zero
	lda	#1
zero:
	rts
