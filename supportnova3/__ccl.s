;
;	Long compares
;
	.export	f__cceql
	.export	f__ccnel
	.export f__ccltequl
	.export f__ccgtul
	.export f__ccgtequl
	.export f__ccltul

	.export f__cclteql
	.export f__ccgtl
	.export f__ccgteql
	.export f__ccltl

	.code

f__cceql:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
	sub	0,2,szr
	jmp	false,1
	sub	1,3,snr		; test low word
	subzl	1,1,skp		; if z set 1
false:
	sub	1,1		; if nz set 0
	mffp	3
	jmp	@__tmp,0

f__ccnel:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
	sub	0,2,snr		; high differs -> true
	sub	1,3,szr		; test low, skip if 0
true:
	subzl	1,1
	; result is now 1 or was 0 before the skip
	mffp	3
	jmp	@__tmp,0

f__cclteql:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
	movl	0,0
	movcr	0,0
	movl	2,0
	movcr	2,0
	jmp	dolteq,1
f__ccltequl:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
dolteq:
	subz#	0,2,snr
	jmp	lteq2,1
	subz#	0,2,snc
	jmp	true,1
	jmp	false,1
lteq2:
	subz#	1,3,szc
	sub	1,1,skp		; false
	subzl	1,1		; true
	mffp	3
	jmp	@__tmp,0

f__ccgtl:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
	movl	0,0
	movcr	0,0
	movl	2,0
	movcr	2,0
	jmp	dogtl,1
f__ccgtul:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
dogtl:
	subz	0,2,snr
	jmp	gtul2,1
	subz#	2,0,snc
	jmp	true,1
	jmp	false,1
gtul2:
	subz#	3,1,szc
	sub	1,1,skp		; false
	subzl	1,1		; true
	mffp	3
	jmp	@__tmp,0

f__ccltl:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
	movl	0,0
	movcr	0,0
	movl	2,0
	movcr	2,0
	jmp	doltl,1
f__ccltul:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
doltl:
	sub#	2,0,snr
	jmp	ltul2,1
	adcz#	2,0,snc
	jmp	true,1
	jmp	false,1
ltul2:
	adcz#	3,1,snc
	subzl	1,1,skp
	sub	1,1
	mffp	3
	jmp	@__tmp,0

f__ccgteql:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
	movl	0,0
	movcr	0,0
	movl	2,0
	movcr	2,0
	jmp	dogteql,1
f__ccgtequl:
	sta	3,__tmp,0
	popa	2
	popa	3
	lda	0,__hireg,0
dogteql:
	sub#	2,0,snr
	jmp	gtequl2,1
	adcz#	0,2,snc
	jmp	true,1
	jmp	false,1
gtequl2:
	adcz#	1,3,snc
	subzl	1,1,skp
	sub	1,1
	mffp	3
	jmp	@__tmp,0
