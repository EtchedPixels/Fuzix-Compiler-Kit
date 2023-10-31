;
;	memcpy
;
		.export _memcmp
		.code
_memcmp:
		push	bc
		ld	hl,9
		add	hl,sp
		ld	b,(hl)	; Count
		dec	hl
		ld	c,(hl)
		dec	hl
		ld	d,(hl)	; Source 1
		dec	hl
		ld	e,(hl)
		dec	hl
		ld	a,(hl)	; Source 2
		dec	hl
		ld	l,(hl)
		ld	h,a

next:
		ld	a,b
		or	c
		jr	z,ret0

		ld	a,(de)		; get src 2
		cp	(hl)		; check v src 1
		jr	nz, mismatch	; and C if src2 < src1
		inc	de
		inc	hl
		dec	bc
		jr	next
mismatch:
		ld	hl,1
		jr	c, mis_low
		ld	hl,-1
mis_low:
		pop	bc
		ret
ret0:		ld	hl,0
		pop	bc
		ret
