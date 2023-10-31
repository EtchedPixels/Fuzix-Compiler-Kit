;
;	String copy for C
;
		.export _strcpy

_strcpy:
	lxi	h,2
	dad	sp
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a

	; copy from HL to DE, return current DE
	push	d

copy:
	mov	a,m
	stax	d
	inx	h
	inx	d
	ora	a
	jnz	copy

	pop	h
	ret
	