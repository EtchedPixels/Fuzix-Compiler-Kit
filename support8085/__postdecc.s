;
;		TOS = lval of object HL = amount
;
		.export __postdecc

		.setcpu 8085
		.code
__postdecc:
		xchg
		pop	h
		shld	__retaddr	; save return
		pop	h
		mov	a,m
		mov	d,a
		sub	e
		mov	m,a
		mov	e,d
		jmp	__ret
