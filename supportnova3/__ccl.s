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
	popa	3
	popa	2
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
	popa	3
	popa	2
	lda	0,__hireg,0
	sub	2,0,snr		; high differs -> true
	sub	3,1,szr		; test low, skip if 0
	; At this point if we are equal AC1 is already 0
true:
	subzl	1,1
	; result is now 1 or was 0 before the skip
	mffp	3
	jmp	@__tmp,0

f__cclteql:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
	movl	0,0
	movcr	0,0
	movl	2,2
	movcr	2,2
	jmp	dolteq,1
f__ccltequl:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
dolteq:
	sub#	2,0,snr		; skip if A != B
	jmp	lteq2,1
	subz#	2,0,szc		; skip if A > B
	jmp	true,1		; high word is < so true
	jmp	false,1		; high woird is > so false
lteq2:
	subz#	3,1,snc		; skip if A <= B
	sub	1,1,skp		; false
	subzl	1,1		; true
	mffp	3
	jmp	@__tmp,0

f__ccgtl:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
	movl	0,0
	movcr	0,0
	movl	2,2
	movcr	2,2
	jmp	dogtl,1
f__ccgtul:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
dogtl:
	sub#	2,0,snr		; skip if  A != B
	jmp	gtul2,1		; handle equality in top word
	subz#	2,0,snc		; skip if A <= B
	jmp	true,1		; was > so we are good
	jmp	false,1		; was < so we are false
gtul2:
	subz#	3,1,szc		; skip if A > B
	sub	1,1,skp		; false as A <= B
	subzl	1,1		; true
	mffp	3
	jmp	@__tmp,0

f__ccltl:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
	movl	0,0
	movcr	0,0
	movl	2,2
	movcr	2,2
	jmp	doltl,1
f__ccltul:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
doltl:
	sub#	2,0,snr		; skip if A != B
	jmp	ltul2,1		; compare lower word
	adcz#	2,0,snc		; skip if A < B
	jmp	false,1		; was >= so false
	jmp	true,1		; was < so true
ltul2:
	adcz#	3,1,snc		; skip if A < B
	sub	1,1,skp		; set false as >=
	subzl	1,1		; else set true
	mffp	3
	jmp	@__tmp,0

f__ccgteql:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
	movl	0,0
	movcr	0,0
	movl	2,2
	movcr	2,2
	jmp	dogteql,1
f__ccgtequl:
	sta	3,__tmp,0
	popa	3
	popa	2
	lda	0,__hireg,0
dogteql:
	sub#	2,0,snr		; skip if A != B
	jmp	gtequl2,1	; consider lower word
	adcz#	2,0,snc		; skip if A < B
	jmp	true,1		; was >= so true
	jmp	false,1		; was < so false
gtequl2:
	adcz#	3,1,snc		; skip if A < B
	subzl	1,1,skp		; set true as >=
	sub	1,1		; else set false
	mffp	3
	jmp	@__tmp,0
