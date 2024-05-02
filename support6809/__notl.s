	.export __notl
	.code
__notl:
	cmpy	#0
	bne	false
	cmpd	#0
	beq	true
false:
	clra
	clrb
	rts
true:
	; D is zero
	incb
	rts
