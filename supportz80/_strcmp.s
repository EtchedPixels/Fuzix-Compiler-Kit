;
;	strncmp
;
		.export _strcmp
		.code
_strcmp:
		ld	hl,7
		add	hl,sp
		ld	d,(hl)	; Source 1
		dec	hl
		ld	e,(hl)
		dec	hl
		ld	a,(hl)	; Source 2
		dec	hl
		ld	l,(hl)
		ld	h,a

next:
		ld	a,(de)		; get src 2
		cp	(hl)		; check v src 1
		jr	nz, mismatch	; and C if src2 < src1

		or	a		; matched end of string ?
		jr	z, ret0
		inc	de
		inc	hl
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
