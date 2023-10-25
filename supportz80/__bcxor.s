		.export __bcxra
		.export __bcxrac
		.code

__bcxra:
		; BC &= HL
		ld a,h
		xor b
		ld b,a
__bcxrac:
		ld a,l
		xor c
		ld c,a
		ret
