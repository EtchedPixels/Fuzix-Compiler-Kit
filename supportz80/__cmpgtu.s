		.export __cmpgtu
		.export __cmpgtub
		.code

		; true if HL > DE
__cmpgtu:
		or	a
		sbc	hl,de
		jp	c,__false
		jp	nz,__true
		jp	__false
