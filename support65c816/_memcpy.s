	.65c816
	.a16
	.i16

	.export _memcpy

_memcpy:	; this one is ugly as we have Y as our stack
	lda	0,y	; dest
	sta	@tmp
	lda	2,y	; src
	sta	@tmp2
	lda	4,y
	and	#1
	beq	words
	rep	#0x20
	.a8
	lda	(@tmp)
	sta	(@tmp2)
	sep	#0x20
	.a16
	inc	@tmp
	inc	@tmp2
words:
	lda	4,y
	lsr	a
	tax
next:
	lda	(@tmp)
	sta	(@tmp2)
	inc	@tmp
	inc	@tmp2
	inc	@tmp
	inc	@tmp2
	dex
	bne	next
	lda	0,y
	jmp	__fnexit6
