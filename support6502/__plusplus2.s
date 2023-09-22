;
;	XA is the pointer, add the amount given. These are used a lot
;

	.globl	__plusplus2
	.text

__plusplus2:
	sta	@tmp
	stx	@tmp+1
	ldy	#1
	ldx	(@tmp),y
	dey
	lda	(@tmp),y
	inc	(@tmp),y
	beq	l1
	iny
	inc	(@tmp),y
	dey
l1:	inc	(@tmp),y
	beq	l2
	iny
	inc	(@tmp),y
	dey
l2:	rts		; exits with y = 0
