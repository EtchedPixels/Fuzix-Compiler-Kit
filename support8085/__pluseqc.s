;
;		TOS = lval of object L = amount
;
		.export __pluseqc
		.export __plusequc

		.setcpu 8085
		.code
__pluseqc:
__plusequc:
		xchg
		pop	h
		shld	__retaddr	; save return
		pop	h
		mov	a,m
		add	e
		mov	m,a
		mov	l,a
		jmp	__ret
