;
;	32bit add of TOS and hireg:1
;
	.export __plusl
	.code
__plusl:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
	addz	3,1,szc
	inc	0,0
	add	2,0
	sta	0,__hireg,0
	mffp	3
	jmp	@__tmp
