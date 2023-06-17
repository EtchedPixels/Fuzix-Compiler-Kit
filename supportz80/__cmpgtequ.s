		.export __cmpgtequ
		.export __cmpgtequb
		.code

		; true if HL >= DE

__cmpgtequ:
		or	a
		sbc	hl,de
		jp	c,__false
		jp	__true
__cmpgrequb:
		ld	a,l
		cp	e
		jp	c,__false
		jp	__true