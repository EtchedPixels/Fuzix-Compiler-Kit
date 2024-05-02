	.export __remul
	.code
	.setcpu 6803
__remul:
	pshb
	psha
	ldd	@hireg
	pshb
	psha
	tsx
	jsr	div32x32
	; Result is in tmp2/tmp3
	ldd	@tmp2
	std	@hireg
	ldd	@tmp3
	pulx
	pulx
	pulx
	ins
	ins
	ins
	ins
	jmp	,x
