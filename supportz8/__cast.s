	.export __castc_
	.export __castc_l
	.export __cast_l

	.code

; We have no sex instructions so we have to do this the hard way

__castc_:
__castc_l:
	cp r3,#0x80
	jr nc, negc
	clr r2
	clr r1
	clr r0
	ret
negc:
	ld r2,#0xFF
	ld r1,r2
	ld r0,r1
	ret

__cast_l:
	cp r2,#0x80
	jr nc, negw
	clr r0
	clr r1
	ret
negw:
	ld r1,#0xFF
	ld r0,r1
	ret
