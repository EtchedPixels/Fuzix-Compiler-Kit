;
;		TOS = lval of object HL = amount
;
		.export __minuseq

		.setcpu 8085
		.code
__minuseq:
		shld	__tmp		; save add value
		pop	h
		shld	__retaddr	; save return
		pop	d
		push	d		; get a copy of the TOS address
		lhlx			; load it into HL
		xchg
		lhld	__tmp
		mov	a,e
		sub	l
		mov	l,a
		mov	a,d
		sub	h
		mov	h,a
		
		pop	d		; get the TOS address
		shlx			; store it back
		jmp	__ret
