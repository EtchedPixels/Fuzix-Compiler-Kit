;
;	String copy for C
;
		.export _strcpy

_strcpy:
	ld	hl,2
	add	hl,sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a

	; copy from HL to DE, return current DE
	push	de

copy:
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	de
	or	a
	jr	nz, copy

	pop	hl
	ret
