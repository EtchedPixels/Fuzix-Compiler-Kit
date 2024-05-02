	.export __notl
	.code
	.setcpu 6803
__notl:
	orab	@hireg
	orab	@hireg+1
	subd	#0
	beq	true
	clra
	clrb
	rts
true:
	incb		; was already 0
	rts
