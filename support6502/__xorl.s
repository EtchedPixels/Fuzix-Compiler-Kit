;
;	TOS ^ hireg:XA
;
	.export __xorl

__xorl:
	jsr __pop32
	eor @tmp
	pha
	txa
	eor @tmp+1
	tax
	lda @tmp1
	eor @hireg
	sta @hireg
	lda @tmp1+1
	eor @hireg+1
	sta @hireg+1
	pla
	rts
