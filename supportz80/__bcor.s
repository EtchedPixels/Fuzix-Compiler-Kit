		.export __bcora
		.export __bcorac
		.code

__bcora:
		; BC &= HL
		ld a,h
		or b
		ld b,a
__bcorac:
		ld a,l
		or c
		ld c,a
		ret
