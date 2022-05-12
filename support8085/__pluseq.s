;
;		TOS = lval of object HL = amount
;
		.export __pluseq
		.export __plusequ

		.setcpu 8085
		.code
__pluseq:
__plusequ:
		shld	__tmp		; save add value
		pop	h
		shld	__retaddr	; save return
		pop	d
		push	d		; get a copy of the TOS address
		lhlx			; load it into HL
		xchg
		lhld	__tmp
		dad	d		; add __tmp to it
		pop	d		; get the TOS address
		shlx			; store it back
		jmp	__ret
