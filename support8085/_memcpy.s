;
;	memcpy
;
		.export _memcpy
		.setcpu 8085
		.code
_memcpy:
	push	b
	ldsi	8	; High byte of count
	xchg
	mov	b,m
	dcx	h
	mov	c,m
	dcx	h
	mov	e,m	; Source
	dcx	h
	mov	d,m
	dcx	h
	mov	a,m	; Destination
	dcx	h
	mov	l,m
	mov	h,a

	mov	a,c
	ora	b
	jz	done

	push	h
loop:
	ldax	d
	inx	d
	mov	m,a
	inx	h
	dcx	b
	jnk	loop
	pop	d

done:
	pop	b
	ret
