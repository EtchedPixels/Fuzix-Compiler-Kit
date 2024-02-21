	.65c816
	.a16
	.i16

	.export __mulul
	.export __mull
	.export __muleqxl
	.export __muleqxul

;
;	This is fairly basic. It might be faster to do 3 16x16 muls
;

__mull:
__mulul:
	dey		; make a working space
	dey
	dey
	dey
	;
	;	We are now multiplying 4-7,y (arg) with hireg:a
	;
	tyx		; no stz ,y
	stz 0,x		; initialize working result
	stz 2,x

	jsr slice	; do low 16 x 32
	lda @hireg
; TODO	beq done	; 32 x 16 is common in C code so skip high word if
;			; we can
	jsr slice	; do the next 16x32
done:
	lda 2,y
	sta @hireg
	lda 0,y
	jmp __fnexit8

slice:
	ldx #16		; 16bits
	stx @tmp
	tyx		; for the rol addressing
next16:
	ror a		; check if the bit is set
	bcc noadd	; if not we just shift
	pha
	lda 0,y
	clc
	adc 4,y		; Add the current value to the working count
	sta 0,y
	lda 2,y	
	adc 6,y
	sta 2,y
	pla
noadd:			; shift arg left (x 2)
	rol 4,x
	rol 6,x
	dec @tmp
	bne next16
	rts

hislice:
	ldx #16		; 16bits
	stx @tmp
	tyx		; for the rol addressing
next16h:
	ror a		; check if the bit is set
	bcc noaddh	; if not we just shift
	pha
	lda 2,y		; we know 4,y is already zero now
	clc
	adc 6,y
	sta 2,y
	pla
noaddh:			; shift arg left (x 2)
	rol 6,x		; 4,y is zero
	dec @tmp
	bne next16h
	rts

__muleqxl:
__muleqxul:
	; x is the ptr
	sta @tmp
	dey
	dey
	dey
	dey
	lda 0,x
	sta 0,y
	lda 2,x
	sta 2,y
	lda @tmp
	phx
	jsr __mull
	; This cleaned up our stack frame for us. Result is in hireg:a
	plx
	pha
	lda 2,x
	sta @hireg
	pla
	rts
