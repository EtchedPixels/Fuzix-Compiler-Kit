;
;		TOS = lval of object HL = mask
;
		.export __andeq
		.export __andequ

		.setcpu 8085
		.code
__andeq:
__andequ:
		shld	__tmp		; save add value
		pop	h
		shld	__retaddr	; save return
		pop	d
		push	d		; get a copy of the TOS address
		lhld	__tmp
		ldax	d
		ana	h
		mov	h,a
		inx	d
		ldax	d
		ana	l
		mov	l,a
		xchg
		pop	h
		mov	e,m
		inx	h
		mov	d,m
		xchg
		jmp	__ret
