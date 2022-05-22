;
;			HL / DE	unsigned
;
		.export __divu
		.export __divdeu
		.export __remu
		.export __remdeu

		.setcpu	8085
		.code

__divu:
	xchg
	pop	h
	xthl
	call	__divdeu
	xchg
	ret

__divdeu:
	call	__remdeu
	xchg
	ret

__remu:
	xchg
	pop	h
	xthl
__remdeu:
	push	b

	mov	b,h
	mov	c,l

	lxi	h,17
	push	h
	mov	l,h		; cheap lxi h,0
	push	h

div_loop:
	inx	sp		; dec counter on stack
	xthl			; ugly as we don't quite have enough
	dcr	h		; registers
	xthl
	jz	done

	xthl
	dad	h
	xthl

	dad	h
	xchg
	dad	h
	xchg
	jnc	set0
	inx	h		; safe as we now bit 0 is clear right now
set0:
	; Now digure out if we are subtracting

	mov	a,h
	cmp	b
	jc	div_loop
	jnz	div_loop
	mov	a,l
	cmp	c
	jc	div_loop

	; Ok we want to subtract
	dsub			; for 8080 do it via A

	; Update quotient (on stack top)
	xthl
	inx	h
	xthl
	jmp	div_loop	

done:
	; BC holds division result, HL remainder
	; put it into DE restore BC
	mov	e,c
	mov	d,b
	pop	b
	ret
