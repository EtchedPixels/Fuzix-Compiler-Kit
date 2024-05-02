	.export __notl
	.code
__notl:
	cpy	#0
	bne	true
	subd	#0
	beq	true
	clra
	clrb
	rts
true:	; D was 0 so make it 1 and set the flags
	incb
	rts
