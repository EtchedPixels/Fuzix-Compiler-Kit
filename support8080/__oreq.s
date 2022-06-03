;
;		TOS = lval of object HL = mask
;
		.export __oreq
		.export __orequ

		.setcpu 8080
		.code
__oreq:
__orequ:
		shld	__tmp		; save add value
		pop	h
		shld	__retaddr	; save return
		pop	d
		push	d		; get a copy of the TOS address
		lhld	__tmp
		ldax	d
		ora	h
		mov	h,a
		inx	d
		ldax	d
		ora	l
		mov	l,a
		xchg
		pop	h
		mov	e,m
		inx	h
		mov	d,m
		xchg
		jmp	__ret
