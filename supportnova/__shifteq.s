	.export f__shleq
	.export f__shreq
	.export f__shrequ

	.export f__shleql
	.export f__shreql
	.export f__shrequl

	.code

f__shleq:
	lda	2,__sp,0
	lda	2,0,2		;	pointer
	dsz	sp
	neg	1,0
	lda	1,0,2
	mov#	0,0,snr
	jmp	out,1
shllp:
	movzl	1,1
	inc	0,0,szr
	jmp	shllp,1
done:	sta	1,0,2
out:	sta	3,__tmp,0
	lda	3,__fp,0
	jmp	@__tmp,0

f__shrequ:
	lda	2,__sp,0
	lda	2,0,2		;	pointer
	dsz	sp
	neg	1,0
	lda	1,0,2
	mov#	0,0,snr
	jmp	out,1
shrulp:
	movzr	1,1
	inc	0,0,szr
	jmp	shrulp,1
	jmp	done,1

f__shreq:
	lda	2,__sp,0
	lda	2,0,2		;	pointer
	dsz	sp
	neg	1,0
	lda	1,0,2
	mov#	0,0,snr
	jmp	out,1
	movl#	1,1,snc
	jmp	shrulp,1
shrlp:	movor	1,1
	inc	0,0,szr
	jmp	shrlp,1
	jmp	done,1

f__shleql:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2		;	pointer
	dsz	sp
	neg	1,0
	lda	3,0,2
	lda	1,1,2
	mov#	0,0,snr
	jmp	lout,1
lshllp:
	movzl	1,1
	movl	3,3
	inc	0,0,szr
	jmp	lshllp,1
ldone:
	sta	3,0,2
	sta	1,1,2
lout:
	sta	3,__hireg,0
	lda	3,__fp,0
	jmp	@__tmp,0

f__shrequl:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2		;	pointer
	dsz	sp
	neg	1,0
	lda	3,0,2
	lda	1,1,2
	mov#	0,0,snr
	jmp	lout,1
lshrulp:
	movzr	3,3
	movr	1,1
	inc	0,0,szr
	jmp	lshrulp,1
	jmp	ldone,1

f__shreql:
	sta	3,__tmp,0
	lda	2,__sp,0
	lda	2,0,2		;	pointer
	dsz	sp
	neg	1,0
	lda	3,0,2
	lda	1,1,2
	mov#	0,0,snr
	jmp	lout,1
	movl#	3,3,snc
	jmp	lshrulp,1
lshrlp:
	movor	3,3
	movr	1,1
	inc	0,0,szr
	jmp	lshrlp,1
	jmp	ldone,1
