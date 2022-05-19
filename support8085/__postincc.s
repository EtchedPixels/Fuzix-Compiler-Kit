;
;		TOS = lval of object HL = amount
;
		.export __postincc

		.setcpu 8085
		.code
__postincc:
		xchg
		pop	h
		shld	__retaddr	; save return
		pop	h
		mov	a,m
		mov	d,a
		add	e
		mov	m,a
		mov	e,d
		jmp	__ret
