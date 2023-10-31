;
;	String copy for C
;
		.export _strlcpy

_strlcpy:
	push	bc	; save register variable BC
	ld	hl,9
	add	hl,sp
	ld	c,(hl)
	dec	hl
	ld	b,(hl)
	dec	hl
	ld	e,(hl)
	dec	hl
	ld	d,(hl)
	dec	hl
	ld	a,(hl)
	dec	hl
	ld	l,(hl)
	ld	h,a

	; copy from DE to HL limit length BC
	push	b	; limit size - needed for return value
copy:
	ld	a,(de)
	ld	(hl),a
	inc	hl
	inc	de
	or	a
	jr	z, done
	dec	bc
	ld	a,b
	or	c
	jr	nz, copy
	; Hit the end
done:
	pop	hl
	ld	a,l
	sub	a,c
	ld	l,a
	ld	a,h
	sbc	a,b
	ld	h,a
	pop	bc	; and recover register variable BC
	ret
