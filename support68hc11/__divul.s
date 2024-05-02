	.export __divul
	.code
__divul:
	pshb
	psha
	pshy
	tsx
	jsr	div32x32
	pulx
	pulx
	pulx
	puly
	pula
	pulb
	jmp	,x
