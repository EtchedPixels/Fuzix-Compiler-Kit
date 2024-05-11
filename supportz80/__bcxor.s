		.export __bcxor
		.export __bcxorc
		.code

__bcxor:
		; BC &= HL
		ld a,h
		xor b
		ld b,a
__bcxorc:
		ld a,l
		xor c
		ld c,a
		ret
