;
;	Compute TOS + HL -> HL
;
		.export __plus
		.export __plusu

		.setcpu 8085
		.code
__plus:
__plusu:
		xchg			; working register into DE
		pop	h		; return address
		shld	__retaddr
		pop	h		; top of stack for maths
		dad	d
		jmp	__ret
