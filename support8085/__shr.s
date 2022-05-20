;
;	Shift the top of stack right by HL (arithemtic)
;
			.export __shr
			.setcpu 8085
			.code
__shr:
		xchg
		pop	h
		shld	__retaddr
		pop	h
shift1:
		arhl
		dcr	e
		jnz	shift1
		jmp	__ret

