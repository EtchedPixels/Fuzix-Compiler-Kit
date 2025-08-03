;
;	TOS & hireg:XA
;
	.export __bandl

__bandl:
	jsr __pop32
	and @tmp
	pha
	txa
	and @tmp+1
	tax
	lda @tmp1
	and @hireg
	sta @hireg
	lda @tmp1+1
	and @hireg+1
	sta @hireg+1
	pla
	rts
