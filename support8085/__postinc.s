;
;		TOS = lval of object HL = amount
;
		.export __postinc

		.setcpu 8085
		.code
__postinc:
		xchg
		pop	h
		shld	__retaddr	; save return
		pop	h
		mov	a,m
		sta	__tmp
		add	e
		mov	m,a
		inx	h
		mov	a,m
		sta	__tmp+1
		adc	d
		mov	m,a
                lhld	__tmp
		jmp	__ret
