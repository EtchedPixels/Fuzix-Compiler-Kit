;
;	*TOS - AC
;
	.export __minuseqc
	.code


__minuseqc:
	call @__pop12
	lda *r13
	sub a,r5
	sta *r13
	mov a,r5
	rets
