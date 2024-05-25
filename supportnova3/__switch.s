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
	lda	3,0,3		; Get switch table
	lda	0,0,3		; get table size
	com	0,0		; as we have inc but not dec
switchn:
	lda	2,1,3		; compare value
	subz#	1,2,snr
	jmp	@2,3
	inc	3,3		; move on a record
	inc	3,3
	inc	0,0,szr
	jmp	switchn,1
	jmp	@1,3

f__switchl:
	lda	3,0,3
	lda	0,0,3
	sta	0,__tmp,0
	lda	0,__hireg,0
switchln:
	lda	2,1,3
	subz#	1,2,snr
	jmp	nextm,1
	lda	2,2,3
	subz#	0,2,snr
	; Matched
	jmp	@3,3		; off we go
nextm:
	inc	3,3
	inc	3,3
	inc	3,3
	dsz	__tmp,0
	jmp	switchln,1
	jmp	@1,3
	