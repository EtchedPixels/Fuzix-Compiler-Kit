;
;	XA is the pointer, add the amount given. These are used a lot
;

	.export	__plusplus1
	.code

__plusplus1:
	sta	@tmp
	stx	@tmp+1		; pointer into @tmp
	ldy	#1
	lda	(@tmp),y	; high byte into X
	tax
	dey
	lda	(@tmp),y	; low byte into A
	pha			; save low
	clc
	adc	#1		; inc low
	sta	(@tmp),y	; put back
	txa			; get high
	adc	#0		; carry into it if needed
	iny
	sta	(@tmp),y	;save high
	pla			; recover value
	rts		; always exits with Y = 1, XA old value

