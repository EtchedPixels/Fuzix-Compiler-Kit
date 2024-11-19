;
;	*TOS + AC
;
	.export __pluseqc
	.code


__pluseqc:
	call @__pop12

	lda *r13
	add r5,a
	mov a,r5
	sta *r13
	rets
