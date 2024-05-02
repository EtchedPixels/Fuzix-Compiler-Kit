;
;	D = TOS / D (unsigned)
;

	.export __divu
	.export __remu
	.export __xdivequ
	.export __xremequ

	.code

__divu:
	tsy
	ldx 2,y			; get top of stack
	jsr div16x16		; D is now quotient
	xgdx
pop2:
	puly
	pulx
	jmp ,y

__remu:
	tsy
	ldx 2,y			; get top of stack
	jsr div16x16		; D is now quotient
	bra pop2

__xdivequ:
	; ,X / D
	pshx
	ldx ,x			; Value
	jsr div16x16
	xgdx			; result now in D
	pulx
	std ,x
	rts

__xremequ:
	; ,X % D
	pshx
	ldx ,x			; Value
	jsr div16x16
	pulx
	std ,x
	rts
