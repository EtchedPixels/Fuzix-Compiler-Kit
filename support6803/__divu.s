;
;	D = TOS / D (unsigned)
;

	.export __divu
	.export __remu
	.export __xdivequ
	.export __xremequ
	.export __pop2

	.code

	.setcpu	6803

__divu:
	tsx
	ldx 2,x			; get top of stack
	jsr div16x16		; D is now quotient
	stx @tmp
	ldd @tmp		;result into D
__pop2:
	pulx
	ins
	ins
	jmp ,x

__remu:
	tsx
	ldx 2,x			; get top of stack
	jsr div16x16		; D is now quotient
	jmp __pop2

__xdivequ:
	; ,X / D
	pshx
	ldx ,x			; Value
	jsr div16x16
	stx @tmp
	ldd @tmp		; result now in D
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
