		.export __bcor
		.export __bcorc
		.code

__bcor:
		; BC &= HL
		ld a,h
		or b
		ld b,a
__bcorc:
		ld a,l
		or c
		ld c,a
		ret
