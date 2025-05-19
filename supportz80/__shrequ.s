;
;	TOS is the lval, hl is the shift amount
;
;
		.export __shrequ
		.code

__shrequ:
		ex	de,hl	; shift into de
		pop	hl
		ex	(sp),hl	; pointer into hl

		ld	a,e	; save shift value

		ld	e,(hl)
		inc	hl
		ld	d,(hl)

		; No work to do
		and	15
		ret	z

		cp	8
		jr	c,nobyte

		ld	e,d
		ld	d,0

		sub	8
nobyte:
		jr	z,shrout
		push	bc
		ld	b,a
shuffle:
		srl	d
		rr	e
		djnz	shuffle

		ld	(hl),d
		dec	hl
		ld	(hl),e

		pop	bc
shrout:
		ex	de,hl
		ret
