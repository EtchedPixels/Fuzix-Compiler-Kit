	.export f__switchl
	.export f__switch
	.export f__switchc

f__switchc:
	; The words will be a byte left packed but otherwise the same
	; due to alignment. Just mask and swap match
	lda	0,N255,0
	ands	0,1		; Mask and swap to high byte
f__switch:
	; Word following call holds the pointer, AC1 holds the match
	lda	2,0,3		; Get switch table
	lda	0,0,2		; get table size
	sta	0,__tmp,0
	lda	3,__fp,0
switchn:
	lda	0,1,2		; compare value
	subz#	1,0,snr
	jmp	@2,2
	inc	2,2		; move on a record
	inc	2,2
	dsz	__tmp,0
	jmp	switchn,1
	jmp	@1,2

f__switchl:
	lda	2,0,3
	lda	0,0,2
	sta	0,__tmp,0
	lda	0,__hireg,0
switchln:
	lda	3,1,2
	subz#	0,3,szr
	jmp	nextm,1
	lda	3,2,2
	subz#	1,3,snr
	; Matched
	jmp	matched,1	; off we go
nextm:
	inc	2,2
	inc	2,2
	inc	2,2
	dsz	__tmp,0
	jmp	switchln,1
	lda	3,__fp,0
	jmp	@1,2
matched:
	lda	3,__fp,0
	jmp	@3,2
