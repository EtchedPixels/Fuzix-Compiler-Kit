;
;		HL holds the pointer
;
		.export __derefl
		.export __dereflsp
		.export __dereff
		.code

__dereflsp:
		add	hl,sp
__dereff:	; Float is the same for our purposes
__derefl:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	hl
		push	de
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		ld	(__hireg),de
		pop	hl
		ret
