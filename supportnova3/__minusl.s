;
;	32bit TOS minus hireg:1
;
	.export f__minusl
	.code
f__minusl:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
	subz	3,1,szc
	sub	2,0,skp
	adc	2,0
	sta	0,__hireg,0
	mffp	3
	jmp	@__tmp
