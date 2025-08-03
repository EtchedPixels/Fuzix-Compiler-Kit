;
;	TOS | hireg:XA
;
	.export __orl

__orl:
	jsr __pop32
	ora @tmp
	pha
	txa
	ora @tmp+1
	tax
	lda @tmp1
	ora @hireg
	sta @hireg
	lda @tmp1+1
	ora @hireg+1
	sta @hireg+1
	pla
	rts
