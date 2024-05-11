		.export __bcand
		.export __bcandc
		.code

__bcand:
		; BC &= HL
		ld a,h
		and b
		ld b,a
__bcandc:
		ld a,l
		and c
		ld c,a
		ret
