;
;	TOS is the lval, hl is the shift amount
;
;
		.export __shleq
		.setcpu 8085
		.code

__shleq:
	xchg		; shift into de
	pop	h
	xthl		; pointer into hl

	xchg		; pointer into de, shift into hl

	mov	a,l
	ani	15
	adi	<shiftit
	mov	l,a
	mov	h,a
	aci	>shiftit
	mov	a,h
	push	h
	lhlx		; load the value
	ret		; into the unroll
	
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
	dad	h
shiftit:
	shlx
	ret
