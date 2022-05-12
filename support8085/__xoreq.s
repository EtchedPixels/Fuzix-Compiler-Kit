;
;		TOS = lval of object HL = mask
;
		.export __xoreq
		.export __xorequ

		.setcpu 8085
		.code
__xoreq:
__xorequ:
		shld	__tmp		; save add value
		pop	h
		shld	__retaddr	; save return
		pop	d
		push	d		; get a copy of the TOS address
		lhld	__tmp
		ldax	d
		xra	h
		mov	h,a
		inx	d
		ldax	d
		xra	l
		mov	l,a
		xchg
		pop	h
		mov	e,m
		inx	h
		mov	d,m
		xchg
		jmp	__ret
