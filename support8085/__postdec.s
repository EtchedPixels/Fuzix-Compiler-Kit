;
;		TOS = lval of object HL = amount
;
		.export __postdec

		.setcpu 8085
		.code
__postdec:
		xchg
		pop	h
		shld	__retaddr	; save return
		pop	h
		mov	a,m
		sta	__tmp
		sub	e
		mov	m,a
		inx	h
		mov	a,m
		sta	__tmp+1
		sbc	d
		mov	m,a
                lhld	__tmp
		jmp	__ret
