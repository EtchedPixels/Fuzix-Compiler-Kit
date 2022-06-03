			.export __ldwordw
			.setcpu 8080
			.code

__ldwordw:
	pop	h		; return address
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	push	h
	dad	sp		; turn offset into address
	mov	e,m
	inx	h
	mov	d,m
	xchg			; into d
	ret
