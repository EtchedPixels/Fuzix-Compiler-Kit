;
;	memcpy
;
		.export _memcpy
		.code
_memcpy:
		push	bc
		ld	h,9
		add	hl,sp
		mov	b,(hl)
		dec	hl
		mov	c,(hl)
		dec	hl
		mov	d,(hl)	; Source
		dec	hl
		mov	e,(hl)
		dec	hl
		mov	a,(hl)	; Destination
		dec	hl
		mov	l,(hl)
		mov	h,a

		mov	a,b
		or	c
		jr	z,done

		push	hl
		ex	de,hl
		ldir
		pop	hl
done:
		pop	bc
		ret
