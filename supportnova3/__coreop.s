;
;	Core support functions. As much because of the constant loading
;	joy of the Nova as for anything else
;
	.export f__jf
	.export f__jt
	.export f__booljf
	.export f__booljt
	.export f__notjf
	.export f__notjt
	.export f__const1
	.export f__const1l
	.export f__const0
	.export f__const0l
	.export f__iconst1
	.export f__iconst1l
	.export f__sconst1
	.export f__sconst1l
	.export f__cpush
	.export f__cpushl
	.export f__cipush
	.export f__cipushl

	.code

f__notjt:
	sub	1,1,snr		; skip if value is non zero
	subzl	1,1,skp		; was 0 value so set 1
	sub	1,1		; else set 0
	jmp	f__jt,1
f__booljt:
	sub	1,1,szr		; if it was 0 skip (stays 0)
	subzl	1,1		; set 1
f__jt:	; AC3 now holds our return ptr
	inc	3,3
	mov#	1,1,szr
	lda	3,-1,3	; get new address to use
	jmp	out,1

f__notjf:
	sub	1,1,snr		; skip if value is non zero
	subzl	1,1,skp
	sub	1,1
	jmp	f__jf,1
f__booljf:
	sub	1,1,szr		; if it was 0 skip (stays 0)
	subzl	1,1		; set 1
f__jf:
	inc	3,3
	mov#	1,1,snr
	lda	3,-1,3	; get new address to use
	jmp	out,1


f__const1l:
	lda	1,0,3
	sta	1,__hireg,0
	inc	3,3
f__const1:
	lda	1,0,3
outi:
	inc 3,3
out:
	sta	3,__tmp,0
	mffp	3
	jmp	@__tmp,0

f__const0l:
	lda	0,0,3
	sta	0,__hireg,0
	inc	3,3
f__const0:
	lda	0,0,3
	jmp	outi,1

f__iconst1l:
	lda	2,0,3
	lda	1,0,2
	sta	1,__hireg,0
	lda	1,1,2
	jmp	outi,1

f__iconst1:
	lda	2,0,3
	lda	1,0,2
	jmp	outi,1

f__iconst0l:
	lda	2,0,3
	lda	0,0,2
	sta	0,__hireg,0
	lda	0,1,2
	jmp	outi,1

f__iconst0:
	lda	2,0,3
	lda	1,0,2
	jmp	outi,1

f__sconst1l:
	lda	2,0,3
	lda	0,__hireg,0
	sta	0,0,2
	sta	1,1,2
	jmp	outi,1

f__sconst1:
	lda	2,0,3
	sta	1,0,2
	jmp	outi,1

f__cpush:
	lda	1,0,3
	psha	1
	jmp	@outi,1

f__cpushl:
	lda	0,0,3
	inc	3,3
	lda	1,0,3
	psha	1
	psha	0
	jmp	@outi,1

f__cipush:
	lda	2,0,3
	lda	1,0,2
	psha	1
	jmp	@outi,1

f__cipushl:
	lda	2,0,3
	lda	0,0,2
	lda	1,1,2
	psha	1
	psha	0
	jmp	@outi,1
