
	.export __switchc
;
;	On entry @tmp points to the switch table, A holds the value
;	Table entries are biased by 1 for the rts trick
;
__switchc:
	ldy #0
	pha
	lda (@tmp),y		; count of entries
	sta @tmp2
	beq default		; empty switch
	pla
	iny
next:
	cmp (@tmp),y
	beq found
	tax
	lda #3
	clc
	adc @tmp
	sta @tmp
	bcc l1
	inc @tmp+1
l1:
	dec @tmp2
	bne next
	; No matches - take default
	dey
default:
found:
	iny			; skip match
	iny			; high byte first
	lda (@tmp),y
	pha			; stack it
	dey
	lda (@tmp),y		; low byte
	pha			; stack it
	rts			; jump

