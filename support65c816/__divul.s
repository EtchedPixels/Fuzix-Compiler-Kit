	.65c816
	.a16
	.i16

	.export __divul
	.export __remul
	.export __diveqxul
	.export __remeqxul

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

__divul:
	jsr divsetup
	jsr div32x32
	lda 6,y
	sta @hireg
	lda 4,y
	jmp __fnexit8

__remul:
	jsr divsetup
	jsr div32x32
	lda @tmp3
	sta @hireg
	lda @tmp2
	jmp __fnexit8


;
;	32bit ops but with a 16bit left so we go via x
;
divsetupx:
	dey
	dey
	dey
	dey
	dey
	dey
	dey
	dey
	sta 0,y
	lda @hireg
	sta 2,y
	lda 0,x
	sta 4,y
	lda 2,x
	sta 6,y
	rts

__diveqxul:
	phy
	jsr divsetupx
	phx
	jsr div32x32
	plx
	lda 6,y
	sta @hireg
	lda 4,y
	ply
	rts

__remeqxul:
	phy
	jsr divsetupx
	phx
	jsr div32x32
	plx
	lda @tmp3
	sta @hireg
	lda @tmp2
	ply
	rts
