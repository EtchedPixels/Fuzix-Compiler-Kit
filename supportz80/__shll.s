		.export __shll
		.code

__shll:
		ld	a,l		; shift amount
		pop	hl		; return address
		pop	de		; low word
		ex	(hl),sp		; return address back, get high word
		ex	de,hl		; turn into DE:HL 32bit

		and	31
		jr	z,done

		; Shift DEHL left by A

		cmp	24
		jr	c,not3byte
		ld	d,l
		ld	e,0
		ld	h,e
		ld	l,e
		sub	24
		jr	leftover
not3byte:	cmp	16
		jr	c,not2byte
		xchg
		lxi	h,0
		sub	16
		jr	leftover
not2byte:	cmp	8
		jr	c,remainder
		ld	d,e
		ld	e,h
		ld	h,l
		ld	l,0
		sub	8
leftover:	jr	z,done
remainder:
		push	bc
		ld	b,a
shiftloop:
		add	hl,hl
		ex	de,hl
		adc	hl,hl
		ex	de,hl
		djnz	shiftloop
		pop	bc
done:
		ld	(__hireg),de
		ret
