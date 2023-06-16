;
;		H holds the pointer
;
		.export __derefl
		.code

__derefl:
		push	bc
		ld	c,(hl)
		inc	hl
		ld	b,(hl)
		inc	hl
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		ld	(__hireg),de
		ld	l,c
		ld	h,b
		pop	bc
		ret
