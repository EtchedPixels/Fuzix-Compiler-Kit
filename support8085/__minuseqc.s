;
;		TOS = lval of object L = amount
;
		.export __minuseqc
		.export __minusequc

		.setcpu 8085
		.code
__minuseqc:
__minusequc:
		xchg
		pop	h
		shld	__retaddr	; save return
		pop	h
		mov	a,m
		sub	e
		mov	m,a
		mov	l,a
		jmp	__ret
