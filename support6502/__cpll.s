;
;	Complement hireg:XA
;
	.export __cpll

__cpll:
	pha
	txa
	eor #0xFF
	tax
	lda @hireg
	eor #0xFF
	sta @hireg
	lda @hireg+1
	eor #0xFF
	sta @hireg+1
	pla
	eor #0xFF
	rts
