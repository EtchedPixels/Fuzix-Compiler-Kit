
	.export _abs

_abs:
	pop	de
	pop	hl
	push	hl
	push	de
	bit	7,h
	ret	z
	jp	__negate

