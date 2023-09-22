;
;	XA is the pointer, add the amount given. These are used a lot
;

	.globl	__plusplus1
	.text

__plusplus1:
	sta	@tmp
	stx	@tmp+1
	ldy	#1
	ldx	(@tmp),y
	dey
	lda	(@tmp),y
	inc	(@tmp),y
	beq	l1
	inc	(@tmp),y
l1:	rts		; alwayus exits with Y = 0, AX old value

