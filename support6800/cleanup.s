	.export __cleanup
	.export __cleanupb
	.export __cleanup1
	.export __cleanup2
	.export __cleanup3
	.export __cleanup4
	.export __cleanup5
	.export __cleanup6
	.export __cleanup7
	.export __cleanup8

	.code

__cleanup8:
	tsx
	ldx	0,x
	ins
	ins
	ins
	ins
	bra	__cleanup4s
__cleanup7:
	tsx
	ldx	0,x
	ins
	ins
	ins
	bra	__cleanup4s
__cleanup6:
	tsx
	ldx	0,x
	ins
	ins
	bra	__cleanup4s
__cleanup5:
	tsx
	ldx	0,x
	ins
	bra	__cleanup4s
__cleanup3:
	tsx
	ldx	0,x
	ins
	ins
	ins
	ins
	ins
	jmp 0,x	
__cleanup4:			; 4 is common, 3 is not
	tsx
	ldx	0,x
__cleanup4s:
	ins
	ins
	ins
	ins
	ins
	ins
	jmp 0,x	
__cleanup:
	rts
__cleanup1:
	tsx
	ldx	0,x
	ins
	ins
	ins
	jmp 0,x
__cleanup2:
	tsx
	ldx	0,x
	ins
	ins
	ins
	ins
	jmp 0,x
__cleanupb:
	stab @tmp
	tsx
	ldx 0,x
	ldab 1,x
	ins
	ins
	tsx
	ldx 0,x
__cleanupbs:			; FIXME: could be very slow in extreme cases
	ins
	decb
	bne __cleanupbs
	ins
	ins
	ldab	@tmp
	jmp 0,x
