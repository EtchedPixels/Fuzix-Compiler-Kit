;
;	String scan for C
;
		.export _strrchr

_strrchr:
	push	bc
	ld	hl,4
	add	hl,sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	l,(hl)	; symbol we want
	ex	de,hl
	ld	bc,0	; Return NULL by default

next:
	ld	a,(hl)
	or	a
	jr	z, end
	cp	e
	jr	nz, nomatch
	; Save the match
	ld	b,h
	ld	c,l
	; Keep scanning as we want the right most
nomatch:
	inc	hl
	jp	next
end:
	ld	l,c
	ld	h,b
	pop	bc
	ret
