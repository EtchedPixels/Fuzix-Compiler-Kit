;
;		Long or, values are on stack and in hireg:h
;
		.export __orl
		.code

__orl:
		ex	de,hl		; pointer into de
		ld	h,2
		add	hl,sp		; so hl is the memory pointer, de the value
		ld	a,(hl)
		or	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		or	d
		ld	d,a
		inc	hl
		push	de
		ld	de,(__hireg)
		ld	a,(hl)
		or	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		or	d
		ld	d,a
		ld	(__hireg),de
		pop	hl		; result
		pop	de
		pop	af
		pop	af
		push	de
		ret
