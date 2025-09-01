
	.export __switch
;
;	On entry @tmp points to the switch table, XA holds the value
;	Table entries are biased by 1 for the rts trick
;
__switch:
	ldy #0
	sta @tmp2
	stx @tmp2+1
	lda (@tmp),y		; count of entries
	tax
	beq default		; empty switch
next:
	ldy #1
	lda (@tmp),y
	iny
	cmp @tmp2
	bne nomatch
	lda (@tmp2),y
	cmp @tmp2+1
	beq found
nomatch:
	lda #4
	clc
	adc @tmp
	sta @tmp
	bcc l1
	inc @tmp+1
l1:
	dex
	bne next
	; No matches - take default
	dey
	dey
default:
found:
	iny			; skip match byte 2 (or count)
	iny			; high byte first
	lda (@tmp),y
	pha			; stack it
	dey
	lda (@tmp),y		; low byte
	pha			; stack it
	rts			; jump

