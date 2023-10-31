;
;	strlen
;
		.export _strlen
		.code

_strlen:
		pop	de
		pop	hl
		push	hl
		push	de
		push	bc
		xor	a
		ld	b,a
		ld	c,a
		cpir
		ld	hl,-1
		sbc	hl,bc	; C is always clear here
		pop	bc
		ret
