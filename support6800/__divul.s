	.export __divul
	.code

__divul:
	pshb
	psha
	ldaa 	@hireg
	ldab	@hireg+1
	pshb
	psha
	tsx
	jsr	div32x32
	ldx	4,x		; return address
	ins
	ins
	ins
	ins
	ins
	ins
	pula
	pulb
	jmp	,x
