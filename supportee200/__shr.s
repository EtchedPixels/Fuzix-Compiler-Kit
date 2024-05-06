	.setcpu	4

	.export __shr8
	.export __shr7
	.export __shr6
	.export __shr5
	.export __shr4
	.export __shr3

	.code

__shr7:
	srr	b
__shr6:
	srr	b
__shr5:
	srr	b
__shr4:
	srr	b
__shr3:
	srr	b
	srr	b
	srr	b
	rsr

; The EE200 can't load a register with a constant without clearing link
; so we can't just clrb rlrb invrb inrb to do this
__shr8:
	xfrb	bh,bl
	slrb	bh	; original sign is now in link
	bl	neg
	clrb	bh	; clear bh
	rsr
neg:
	clrb	bh	; No nice ld bh,0xFF
	dcrb	bh
	rsr
