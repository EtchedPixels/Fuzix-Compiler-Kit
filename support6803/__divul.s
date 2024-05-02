	.export __divul
	.code

	.setcpu 6803
__divul:
	pshb
	psha
	ldd 	@hireg
	pshb
	psha
	tsx
	jsr	div32x32
	pulx
	pulx
	pulx
	ins
	ins
	pula
	pulb
	jmp	,x
