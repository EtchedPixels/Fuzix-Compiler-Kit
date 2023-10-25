		.export __bcana
		.export __bcanac
		.code

__bcana:
		; BC &= HL
		ld a,h
		and b
		ld b,a
__bcanac:
		ld a,l
		and c
		ld c,a
		ret
