;
;	String scan for C
;
		.export _strchr

_strchr:
	ld	hl,2
	add	hl,sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	l,(hl)	; symbol we want
	ex	de,hl

next:
	ld	a,(hl)
	or	a
	jr	z, end
	cp	e
	ret	z		; HL points to match
	inc	hl
	jp	next
end:
	ld	hl,0
	ret
