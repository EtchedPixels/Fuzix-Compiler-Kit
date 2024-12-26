	.export __notl
	.code
__notl:
	cpy	#0
	bne	false
	subd	#0
	beq	true
false:
	clra
	clrb
	rts
true:	incb	; D is 0 at this point so make it true
	rts
