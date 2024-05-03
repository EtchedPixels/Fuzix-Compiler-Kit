;
;	D = TOS / D (unsigned)
;

	.export __divu
	.export __remu
	.export __xdivequ
	.export __xremequ

	.code

__divu:
	tsx
	ldx 2,x			; get top of stack
	jsr div16x16		; D is now quotient
	stx @tmp
	ldaa @tmp		;result into D
	ldab @tmp+1
	jmp __pop2

__remu:
	tsx
	ldx 2,x			; get top of stack
	jsr div16x16		; D is now quotient
	jmp __pop2

__xdivequ:
	; ,X / D
	stx @tmp4
	ldx ,x			; Value
	jsr div16x16
	stx @tmp
	ldaa @tmp		; result now in D
	ldab @tmp+1
	ldx @tmp4
	staa ,x
	stab 1,x
	rts

__xremequ:
	; ,X % D
	stx @tmp4
	ldx ,x			; Value
	jsr div16x16
	ldx @tmp4
	staa ,x
	stab 1,x
	rts
