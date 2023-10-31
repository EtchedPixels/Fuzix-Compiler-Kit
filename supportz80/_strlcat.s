;
;	String cat for C
;
		.export _strlcat

_strlcat:
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
	push	bc	; limit size - needed for return value
find:
	ld	a,(hl)
	or	a
	jr	z, copy
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jr	nz, find
	pop	hl	; All space used so return length given
	pop	bc	; Recover register variable
	ret
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
done:
	; Hit the end
	pop	hl
	ld	a,l
	sub	c
	ld	l,a
	ld	a,h
	sbc	a,b
	ld	h,a
	pop	bc	; and recover register variable BC
	ret
