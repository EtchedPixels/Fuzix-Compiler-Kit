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
	ldaa 6,x
	ldab 7,x
	staa @hireg
	staa @hireg+1
	ldaa 8,x
	ldab 9,x
	ldx	4,x		; return address
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	jmp	,x
