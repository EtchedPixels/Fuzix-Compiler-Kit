	.65c816
	.a16
	.i16

	.export __divl
	.export __reml
	.export __diveqxl
	.export __remeqxl

;
;	We handle divide specially to get the stack layout
;	convenient. The backend pushes the left to the data stack
;	instead.
;
divsetup:
	dey
	dey
	dey
	dey
	sta 0,y		; save the working register
	lda @hireg
	sta 2,y
	rts

__divl:
	stz @sign		; clear sign flag
	ldx @hireg		; is the working register negative ?
	bpl signfixed
	jsr __negatel		; negate it if so
	inc @sign		; and count the negation
signfixed:
	jsr divsetup
	lda 6,y			; get the stacked high word
	bpl nocarry		; are we negative
	dec @sign		; remember negation
	eor #0xffff		; negate
	sta 6,y
	lda 4,y
	eor #0xffff
	inc a
	sta 4,y
	bne nocarry
	tyx			; inc doesnt have ,y form so use x
	inc 6,x
nocarry:
	jsr div32x32		; unsigned divide
	lda @sign
	beq nosignfix3		; both same sign - no change
	lda 6,y			; negate into hireg:a
	eor #0xffff
	sta @hireg
	lda 4,y
	eor #0xffff
	inc a
	bne popout		; and done
	inc @hireg		; carried so an extra inc needed
popout:
	jmp __fnexit8		; 4 argument, 4 we added

nosignfix3:
	lda 6,y
	sta @hireg
	lda 4,y
	bra popout


;
;	Same basic idea but the sign is determined solely by hireg
;
__reml:
	; Sign is in hireg
	ldx @hireg		; test sign by throw away store
	bpl msignfixed
	jsr __negatel
msignfixed:
	jsr divsetup
	lda 6,y
	sta @sign		; sign that determines resulting sign
	bpl mnocarry
	eor #0xffff
	sta 6,y
	lda 4,y
	eor #0xffff
	inc a
	sta 4,y
	bne mnocarry
	tyx
	inc 6,x
mnocarry:
	jsr div32x32
	lda @tmp3
	sta @hireg
	lda @tmp2
	ldx @sign
	bpl popout
	jsr __negatel
	bra popout

;
;	A is the pointer. Build the stack and call the main operation
;
;	(A) / hireg:X
;
__diveqxl:
	phx		; save working value
	dey
	dey
	dey
	dey
	tax
	lda 2,x
	sta 2,y
	lda 0,x
	sta 0,y
	pla		; get working value back
	phx
	jsr __divl
eqxlclean:
	; This has removed the 4 bytes of workspace we made on the stack
	plx
	; it did all our cleanup on the data stack
	sta 0,x
	pha
	lda @hireg
	sta 2,x
	pla
	rts

__remeqxl:
	phx		; save working value
	dey
	dey
	dey
	dey
	tax
	lda 2,x
	sta 2,y
	lda 0,x
	sta 0,y
	pla		; get working value back
	phx
	jsr __reml
	bra eqxlclean
