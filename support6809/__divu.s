;
;	D = TOS / D (unsigned)
;

	.export __divu
	.export __remu
	.export __xdivequ
	.export __xremequ
	.export __regdivu
	.export __regmodu

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
	jsr div16x16
	exg d,x			; result now in D
	std [,s++]		; Get pointer back and save
	rts

__xremequ:
	; ,X % D
	stx ,--s		; Save pointer
	ldx ,x			; Value
	jsr div16x16
	std [,s++]		; Get pointer back and save
	rts

__regdivu:
	pshs u
	jsr __divu
	tfr d,u
	rts

__regmodu:
	pshs u
	jsr __remu
	tfr d,u
	rts
