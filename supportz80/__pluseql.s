		.export __pluseql
		.code

__pluseql:
		ex	de,hl
		pop	h
		xthl

		; HL is pointer, hireg:de amount to add

		ld	a,(hl)
		add	e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		adc	d
		ld	(hl),a
		ld	d,a
		inc	hl
		push	d

		ld	de,(__hireg)

		ld	a,(hl)
		adc	e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		adc	d
		ld	(hl),a
		ld	d,a

		ld	(__hireg),de

		pop	hl
		ret
