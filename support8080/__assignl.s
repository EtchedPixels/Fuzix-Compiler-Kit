;
;	Assign the value in hireg:HL to lval at tos.
;
		.export __assignl
		.export	__assign0l
		.setcpu 8080
		.code

__assignl:
		xchg			; hireg:de is our value
		pop	h
		xthl			; hl is now our pointer
		mov	m,e
		inx	h
		mov	m,d
		inx	h
		xchg
		push	d		; for return
		inx	h
		inx	h
		xchg
		lhld	__hireg
		xchg
		mov	m,e
		inx	h
		mov	m,d
		pop	h
		ret


; Assign 0L to lval in HL
__assign0l:
		xra	h
		mov	m,a
		inx	h
		mov	m,a
		inx	h
		mov	m,a
		inx	h
		mov	m,a
		lxi	h,0
		shld	__hireg		; clear hireg
		ret
