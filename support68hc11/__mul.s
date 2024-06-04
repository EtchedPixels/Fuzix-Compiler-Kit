;
;	D = top of stack * D
;
	.export __mul

	.code

__mul:
	tsx
	psha
	pshb
	ldaa 3,x		; low byte
	mul			; D is now low x low
	std @tmp
	ldaa 2,x		; high byte
	pulb
	mul			; D is now high x low
	std @tmp1
	ldaa 3,x		; low byte
	pulb			; high byte of D
	mul			; D is now low x high
	addd @tmp1		; High bytes
	tba			; Shift left 8, discarding
	clrb
	addd @tmp		; Add the low x low
	pulx
	ins
	ins
	jmp ,x

