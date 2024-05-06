	.setcpu	4
	.export	__switchc

	.code

__switchc:
	lda	(x+)	; count
	xfr	al,ah

	bz	found
swent:
	ldab	(x+)
	subb	bl,al
	bz	found
	inx
	inx
	dcr	y
	bnz	swent
found:
	ldx	(x+)
	jmp	(x)



	
