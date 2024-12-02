	.export __notl
	.code

__notl:
	orab	@hireg
	orab	@hireg+1
	bne	false
	tsta
	bne	false1
;	clra			; now AccA,B=0
	incb
	rts
false:
	clrb
false1:
	clra
	rts
