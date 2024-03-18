;
;	D = TOS / D (unsigned)
;

	.export __divu
	.export __remu
	.export __xdivequ
	.export __xremequ
	.code

__divu:
	ldx 2,s			; get top of stack
	jsr div16x16		; D is now quotient
	exg d,x
pop2:
	ldx ,s
	leas 4,s
	jmp ,x

__remu:
	ldx 2,s			; get top of stack
	jsr div16x16		; D is now quotient
	bra pop2

__xdivequ:
	; ,X / D
	stx ,--s		; Save pointer
	ldx ,x			; Value
	exg d,x			; It computes D / X
	jsr div16x16
	exg d,x			; result now in D
	ldx ,s++		; Get pointer back
	std ,x			; Save
	rts

__xremequ:
	; ,X % D
	stx ,--s		; Save pointer
	ldx ,x			; Value
	exg d,x			; It computes D / X
	jsr div16x16
	ldx ,s++		; Get pointer back
	std ,x			; Save
	rts
